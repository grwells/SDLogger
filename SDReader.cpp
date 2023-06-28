#include "SDReader.hpp"

// debug helper function not scoped to SDReader class
// prints the current amount of free memory on the heap
void print_heap_debug(){
    if(USB_DEBUG){
        Serial.print("[DEBUG] memory usage is ");
        double perc = ESP.getFreeHeap();
        Serial.print(perc);
        Serial.println(" bytes");
    }
}

/*
 * Check if any of the filters match the target string
 *  return true if one of them does or filter is "", else return false
 */
bool SDReader::topic_filter_match(vector<string> filter, string target){
    bool check_default = (filter.size() == 1);
    string def = "";

    for(string f : filter){
        if(check_default && f == def){
            return true;

        }else if(f != def && target.find(f) != string::npos){
            return true;
        }
    }
    
    return false;
}

/*
 * Compile collected data into a JSON "page"
 *  which can then be published via the MQTT interface.
 *
 * Returns a single line JSON formatted string.
 */
string SDReader::build_json_page(
        string filename,
        long int epoch,
        long int terminus,
        vector<string> &data
        ){

    string json_data_arr = "[";
    for(string n : data){
        json_data_arr += "\"" + n + "\",";
    }

    json_data_arr += "]";
    
    return "{ \"file name\":"  + filename + 
            ", \"epoch\": " + to_string(epoch) + 
            ", \"terminus\": " + to_string(terminus) +
            ", \"data\": " + json_data_arr + "}";
}



/*
 * Calculate the size, in bytes, of the current page.
 */
int SDReader::calculate_page_size(vector<string> &page){
    int size = 0;
    for(string n : page)
        size += n.size();

    if(USB_DEBUG){
        Serial.print("[DEBUG] memory usage is ");
        double perc = ESP.getFreeHeap();
        Serial.print(perc);
        Serial.println(" bytes");
        Serial.print("[DEBUG] page size is ");
        Serial.print(size);
        Serial.print(" bytes, and capacity is ");
        Serial.println(page.capacity());
    }

    if(USB_DEBUG && size > 34464){
        Serial.println("[WARNING] page size is above MQTT buffer limit of 34464");
    }

    return size;
}

/*
 * Initialize connection to SD card and return false if no connection established.
 */
bool SDReader::initialize_sd_card(){

    const auto ok = this->sd.begin(TT_CLK, TT_MISO, TT_MOSI, TT_SS, &SPI);

    if(!ok){
        Serial.println("[ERROR] failed to initialize sd card");
        return false;
    }
    
    Serial.println("[DEBUG] success initializing sd card reader");
    Serial.print("[DEBUG] bytes free =  ");
    double perc = ESP.getFreeHeap();
    Serial.println(perc);
    return true;
}

/*
 * Read until next newline character into buffer and return as a 
 *  string
 */
string SDReader::read_line(){
    if(!this->file_open) return "";
    
    string buf = "";

    while(this->fp.available()){
        char c = this->fp.read();
        buf += c;

        if(c == '\n'){
            break;
        }
    }

    return buf;
}

/*
 * Collect all log entries within date range and not screened
 *  by the topic filter
 */
void SDReader::read_entry_range_from_files(TimeStamp epoch, 
            TimeStamp terminus, 
            vector<string> topic_filter, 
            int page_length, 
            string prefix, 
            string filetype)
{

    vector<string> data;

    if(this->file_open)
        this->close_file();

    int secs_p_day = 86400;

    long int counter = epoch.get_epoch(); 
    //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    for(int i = epoch.get_epoch(); i < terminus.get_epoch(); i += secs_p_day){
        string mdy = TimeStamp(i).get_mdy(); 
        string test_fn = "/" + prefix + "_" + mdy + "." + filetype;


        this->filename = test_fn;

        if(this->sd.exists(test_fn.c_str())){
            // open and pull data from file
            this->open_file(test_fn);
            //print_heap_debug();
            this->read_entry_range(this->fp, epoch, terminus, topic_filter, page_length);
            this->close_file();

        }else{
            //Serial.println("\tfile doesn't exist");
        }
    }

    /*
    ESP_ERROR_CHECK( heap_trace_stop() );
    heap_trace_dump();
    */
}

/*
 * Collect all log entries in a file not screened by time range or topic filter
 */
void SDReader::read_entry_range(
        File f, 
        TimeStamp epoch, 
        TimeStamp terminus, 
        vector<string> topic_filter,
        int page_length)
{

    // clear log buffer
    vector<string> data = {};
    MQTTMailer mqtt_inst = MQTTMailer::getInstance();

    while(this->fp.available() ){
        //Serial.println("\tmemory usage at top of loop");
        //print_heap_debug();

        if(data.size() < page_length){
            string line = this->read_line();
            if(line.find(":") == string::npos) continue;

            // get line timestamp
            int first_sc = line.find(this->separator);  // first semicolon in line
            int second_sc = line.find(this->separator, first_sc + 1); // second semicolon in line
            string l_ts = line.substr(0, first_sc);     // line timestamp
            string l_topic = line.substr(first_sc + 1, second_sc - first_sc); // line topic
                                                                              //
            // convert to TimeStamp and check if in range
            TimeStamp ts(l_ts);

            if(ts >= epoch && ts <= terminus){
                // in legal time range
                if(this->topic_filter_match(topic_filter, l_topic)){
                    // add line to vector (matches topic filter)
                    data.push_back(line);
                }
            }

        }else{
            // publish page
            // send contents of vector to receiver... then proceed
            TimeStamp epoch(data[0].substr(0, data[0].find(this->separator)));
            TimeStamp terminus(data.back().substr(0, data.back().find(this->separator)));
            string json_page = this->build_json_page(this->filename, epoch.get_epoch(), terminus.get_epoch(), data);

            mqtt_inst.mailMessage(&mqtt_client, string("datagator/data/time_range/") + WiFi.macAddress().c_str(), json_page, false);

            // clear log buffer
            //this->calculate_page_size(data);
            data.clear();           // remove objects from vector (size to 0)
            data.shrink_to_fit();   // shrink memory allocation(capacity) to size of vector
        }
    }
}

