#include <SD.h>
#include "../../include/SDCard.hpp"
#include "SDLogger.hpp"

#include <vector>

/**
 * SDReader provides an interface for opening and
 *  reading data from files.
 */
class SDReader {

    private:

        std::string filename;
        std::string separator = ";";

        SDCard sd;

        bool initialize_sd_card();

    public:

        SDReader(std::string filename);

        SDReader(std::string prefix, 
                int month, 
                int day, 
                int year, 
                std::string filetype);


        std::string read_line();
        std::vector<std::string> read_timestamp_range();

        File open_file();
        void close_file();

};
