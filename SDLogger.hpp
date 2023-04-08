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

        bool initialize_sd_card();

    public:

        SDLogger(){
            filename = "/logfile" + filetype;

            this->initialize_sd_card();
        };

        SDLogger(std::string filename);
        SDLogger(std::string prefix, int month, int day, int year, std::string filetype);

        void write_header(std::vector<std::string> fields);

        void log_absolute_mqtt(std::string time, std::string mqtt_topic, std::string mqtt_message);
        void log_relative_mqtt(std::string time, int offset, std::string mqtt_topic, std::string mqtt_message);

        void write_line(std::string line);
        void append_line(std::string line);

        void read();

        File open_file(const char* mode = FILE_WRITE);
        void close_file();

}; 


