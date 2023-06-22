#include <SD.h>
#include <vector>
#include <string>

#include "../../include/SDCard.hpp"
#include "SDLogger.hpp"
#include "TimeStamp.hpp"
#include "MQTTMailer.hpp"

#ifndef SDREADER_HPP
#define SDREADER_HPP


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

        SDCard sd; // default sd interface passed to constructor

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

        // initialize sd card connection and nothing more
        SDReader(){initialize_sd_card();}

        // initialize sd card and set default file to open
        SDReader(std::string filename){
            this->filename = filename;
            initialize_sd_card();
        }

        // initialize and set the file name structure
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

        // manually set the filename to access
        void set_filename(string filename){this->filename = filename;}

        // read one line as a string from the file
        std::string read_line();

        // retrieve all data within the specified time 
        //  range, potentially accessing mult. files
        void read_entry_range_from_files(TimeStamp epoch, 
                TimeStamp terminus, 
                vector<string> topic_filter, 
                int page_length=5,
                string prefix="log", 
                string filetype="csv");

        // access a single file to retrieve data in time range and 
        //  publish via MQTT
        void read_entry_range(File f, 
                TimeStamp epoch, 
                TimeStamp terminus, 
                vector<string> topic_filter,
                int page_length);

        // open the specified file and get the file pointer
        File* open_file(){
            if(filename == "") return NULL;
            this->fp = (sd.open(this->filename.c_str(), "r"));
            this->file_open = true;
            return &(this->fp);
        }

        // open the specified file from path provided
        File* open_file(string filename){
            this->fp = sd.open(filename.c_str(), "r");
            this->file_open = true;
            return &(this->fp);
        }

        // close the open file
        void close_file(){
            this->file_open = false;
            this->fp.close();
        }

};

#endif
