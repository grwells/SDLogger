/**
 * @file SDLogger.hpp
 * @brief Implements an interface for using SPI, SDLogger, for reading and writing to files on an SD card.
 *
 * @author Garrett Wells
 * @date 2023
 */
#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <SD.h>
#include "../../include/SDCard.hpp"

#include <vector>

#define TT_CLK 18
#define TT_MISO 19
#define TT_MOSI 23
#define TT_SS 13


/**
 * @brief Creates an interface for writing data to a log file
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

        /**
         * @brief Default constructor that performs no initialization.
         */
        SDLogger(){};

        /**
         * @brief Constructor to set the file name
         */
        SDLogger(std::string filename);

        /**
         * @brief SDLogger constructor which takes a prefix + date + filetype based filename.
         */
        SDLogger(std::string prefix, int month, int day, int year, std::string filetype);

        // must be called before logging
        bool initialize_sd_card();

        /**
         * @brief Set the filename to write to the easiest way possible
         */
        void set_filename(std::string);

        /**
         * @brief Slightly more involved but potentially more useful filename
         *  creation
         */
        void set_filename(std::string prefix, int month, int day, int year, std::string filetype);

        /**
         * @brief Writes a header line to a file using append to 
         *  prevent overwriting valuable data.
         */
        void write_header(std::vector<std::string> fields);

        /**
         * @brief Append a line to the target csv file containing 
         *  an absolute time stamp, mqtt topic, and an mqtt message
         */
        void log_absolute_mqtt(std::string time, std::string mqtt_topic, std::string mqtt_message);

        /**
         * @brief Append a line with a relative time stamp to the 
         *  target file
         */
        void log_relative_mqtt(std::string time, int offset, std::string mqtt_topic, std::string mqtt_message);

        /**
         * @brief Write a line to the file, will overwrite the 
         *  contents of the file
         */
        void write_line(std::string line);

        /**
         * @brief Open file and append a line, add newline
         *  if needed
         */
        void append_line(std::string line);

        /**
         * @brief _Not implemented_
         */
        void read();

        /**
         * @brief Opens the file attached to the logger object with the default
         *  mode
         */
        File open_file(const char* mode = FILE_WRITE);

        /**
         * @brief File with name `fn` exists in SD card's filesystem.
         */
        bool exists(std::string fn){return this->sd.exists(fn.c_str());}

        /**
         * @brief File with name stored in `this->filename` exists in SD card's filesystem.
         */
        bool exists(){return this->sd.exists(this->filename.c_str());}

        /**
         * @brief Close card connection.
         */
        void close_card();

}; 

#endif
