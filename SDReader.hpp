/**
 * @file SDReader.hpp
 * @brief Defines the utility class SDReader which is used to  read from SD card files.
 * @author Garrett Wells
 * @date 2023
 */
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
 * @brief SDReader provides an interface for opening and
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

        /**
         * @brief Constructor initializes SD card connection and nothing more.
         */
        SDReader(){initialize_sd_card();}

        /**
         * @brief Constructor initializes SD card and set default file to open.
         *
         * @param[in] filename The default file to open and read data from.
         */
        SDReader(std::string filename){
            this->filename = filename;
            initialize_sd_card();
        }

        /** 
         * @brief Constructor initializes and sets the file name structure.
         *
         * @param[in] prefix What is this file for? Example: `log`.
         * @param[in] month What month did this file start collecting data in?
         * @param[in] day The day starting at?
         * @param[in] year What year is it?
         * @param[in] filetype What filetype/extension is this?
         */
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

        /**
         * @brief Manually set the filename to access.
         *
         * @param[in] filename The name of the file to open. Should be a path if not in the root directory.
         */
        void set_filename(string filename){this->filename = filename;}

        /**
         * @brief Read until next newline character into buffer and return as a 
         *  string.
         */
        std::string read_line();

        /**
         * @brief Retrieve all data within the specified time range, potentially accessing multiple files.
         */
        void read_entry_range_from_files(TimeStamp epoch, 
                TimeStamp terminus, 
                vector<string> topic_filter, 
                int page_length=5,
                string prefix="log", 
                string filetype="csv");

        /**
         * @brief Access a single file to retrieve data in time range and publish via MQTT.
         */
        void read_entry_range(File f, 
                TimeStamp epoch, 
                TimeStamp terminus, 
                vector<string> topic_filter,
                int page_length);

        /**
         * @brief Open the specified file and get the file pointer.
         */
        File* open_file(){
            if(filename == "") return NULL;
            this->fp = (sd.open(this->filename.c_str(), "r"));
            this->file_open = true;
            return &(this->fp);
        }

        /**
         * @brief Open the specified file from path provided.
         */
        File* open_file(string filename){
            this->fp = sd.open(filename.c_str(), "r");
            this->file_open = true;
            return &(this->fp);
        }

        /**
         * @brief Close the open file.
         */
        void close_file(){
            this->file_open = false;
            this->fp.close();
        }

};

#endif
