#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <SD.h>
#include "../../include/SDCard.hpp"

#include <vector>

#define TT_CLK 18
#define TT_MISO 19
#define TT_MOSI 23
#define TT_SS 13


/*
 * Creates an interface for writing data to a log file
 *  on an SD card.
 *
 * A SDLogger is meant to be associated with one file object
 *  for the duration of its lifetime.
 *
 */
class SDLogger {

    private:

        std::string filename;           // name of the file that this SDLogger interacts with
        std::string filetype = ".csv";  // file extension assumed for the file
        std::string separator = ";";    // separator between CSV fields

        SDCard sd;


    public:

        SDLogger(){};

        SDLogger(std::string filename);
        SDLogger(std::string prefix, int month, int day, int year, std::string filetype);

        // must be called before logging
        bool initialize_sd_card();

        void set_filename(std::string);
        void set_filename(std::string prefix, int month, int day, int year, std::string filetype);

        void write_header(std::vector<std::string> fields);

        void log_absolute_mqtt(std::string time, std::string mqtt_topic, std::string mqtt_message);
        void log_relative_mqtt(std::string time, int offset, std::string mqtt_topic, std::string mqtt_message);

        void write_line(std::string line);
        void append_line(std::string line);

        void read();

        File open_file(const char* mode = FILE_WRITE);
        bool exists(std::string fn){return this->sd.exists(fn.c_str());}
        bool exists(){return this->sd.exists(this->filename.c_str());}
        void close_card();

}; 

#endif
