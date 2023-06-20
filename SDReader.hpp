#include <esp_heap_trace.h>
#include <SD.h>
#include <vector>
#include <string>

#include "../../include/SDCard.hpp"
#include "SDLogger.hpp"
#include "TimeStamp.hpp"
#include "MQTTMailer.hpp"


using namespace std;

// must have an mqtt_client connected to publish data
//  to
extern PubSubClient mqtt_client;
extern const bool USB_DEBUG;

/**
 * SDReader provides an interface for opening and
 *  reading data from files created using the SDLogger library.
 */
class SDReader {

    private:

        std::string filename = "";
        std::string separator = ";";

        SDCard sd;

        bool initialize_sd_card();

        bool file_open = false;
        File fp;

        bool topic_filter_match(vector<string> filter, string target);

        string build_json_page(
                string filename,
                long int epoch,
                long int terminus,
                vector<string> &data);

        int calculate_page_size(vector<string> &page);

    public:

        SDReader(){initialize_sd_card();}

        SDReader(std::string filename){
            this->filename = filename;
            initialize_sd_card();
        }

        SDReader(std::string prefix, 
                int month, 
                int day, 
                int year, 
                std::string filetype){

            this->filename = prefix + "_" + 
                to_string(month) + "-" + 
                to_string(day) + "-" + 
                to_string(year) + "." + filetype;

            initialize_sd_card();
        }

        void set_filename(string filename){this->filename = filename;}

        std::string read_line();

        void read_entry_range_from_files(TimeStamp epoch, 
                TimeStamp terminus, 
                vector<string> topic_filter, 
                int page_length=5,
                string prefix="log", 
                string filetype="csv");

        void read_entry_range(File f, 
                TimeStamp epoch, 
                TimeStamp terminus, 
                vector<string> topic_filter,
                int page_length);

        File* open_file(){
            if(filename == "") return NULL;
            this->fp = (sd.open(this->filename.c_str(), "r"));
            this->file_open = true;
            return &(this->fp);
        }

        File* open_file(string filename){
            this->fp = sd.open(filename.c_str(), "r");
            this->file_open = true;
            return &(this->fp);
        }

        void close_file(){
            this->file_open = false;
            this->fp.close();
        }

};

