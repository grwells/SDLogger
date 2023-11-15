/**
 * @file SDReader.cpp
 */
#include "SDReader.hpp"

/**
 * Debug helper function not scoped to SDReader class, prints the current amount of free memory on the heap
 */
void print_heap_debug(){
    if(USB_DEBUG){
        Serial.print("[DEBUG] memory usage is ");
        double perc = ESP.getFreeHeap();
        Serial.print(perc);
        Serial.println(" bytes");
    }
}

/**
 * @brief Check if any of the filters match the target string
 *  return true if one of them does or filter is "", else return false.
 *
 * @param[in] filter
 * @param[in] target
 *
 * @returns `true` if match found, `false` otherwise.
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

/**
 * @brief Compile collected data into a JSON "page"
 *  which can then be published via the MQTT interface.
 *
 * @param[in] filename The name of the file this page came from.
 * @param[in] epoch The beginning timestamp of the range this data was collected from.
 * @param[in] terminus The end timestamp of the range this data was collected from.
 * @param[in] data The vector of data entries collected in this page.
 *
 * @returns A single JSON object string which contains fields: `file name`, `epoch`, `terminus`, and `data`.
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



/**
 * @brief Calculate the size, in bytes, of the current page.
 *
 * If debugging is enabled over serial port, then the page size is printed to the 
 * serial port.
 *
 * @param[in] page Reference to the "page" of collected results.
 *
 * @returns An integer (bytes) representing the size of the page.
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

/**
 * @brief Initialize connection to SD card and return false if no connection established.
 *
 * @returns A boolean value: `true` if initialization successful, `false` otherwise.
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

/**
 * @returns The next line from the file as a string.
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

/**
 * Collect all log entries within date range included by the topic filter. This 
 * search is conducted on all files in the SD card file system. Once a file is found
 * which meets the criteria of time range, prefix, and filetype, `read_entry_range()`
 * is used to parse the file's entries for ones that match time range and topic filter
 * untill the end of the file has been reached. Entries are uploaded to MQTT broker in pages
 * which are limited in length by `page_length`.
 *
 * This process is repeated until all files in the file system have been checked.
 *
 * @param[in] epoch The beginning of the time range to collect data from. 
 * @param[in] terminus The end of the time range to collect data from.
 * @param[in] topic_filter The vector of topics to collect in a page.
 * @param[in] page_length The maximum length of the page to construct.
 * @param[in] prefix Only collect data from files whose prefix matches, for example only collect from `log` files.
 * @param[in] filetype Match file type, this defaults to `csv`.
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

/**
 * Collect all log entries in a file that fall within the time range and match 
 * a topic in topic filter. Collected entries are counted as entries in a "page". Entries are added
 * to the "page" until `page_length` is reached. At this point the entries are uploaded 
 * via MQTT to the broker.
 *
 * This process is repeated until the end of the file is reached or the entries no longer
 * fall within the time range.
 *
 * @param[in] f File object to read from.
 * @param[in] epoch The beginning of the time range to collect entries from.
 * @param[in] terminus The end of the time range to collect entries from.
 * @param[in] topic_filter A vector list of topic strings, which if matching, should be collected.
 * @param[in] page_length The maximum number of results to include in this page.
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

