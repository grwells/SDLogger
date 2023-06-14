#include <SD.h>
#include "../../include/SDCard.hpp"
#include "SDLogger.hpp"
#include "TimeStamp.hpp"

#include <vector>

using namespace std;

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

        File* fp = NULL;

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
                month + "-" + 
                day + "-" + 
                year + "." + filetype;

            initialize_sd_card();
        }

        void set_filename(string filename){this->filename = filename;}

        std::string read_line();

        std::vector<std::string> read_entry_range(TimeStamp epoch, 
                TimeStamp terminus, 
                vector<string> topic_filter);

        File* open_file(){
            if(filename == "") return NULL;

            this->fp = fopen(this->filename, "r");
            return this->fp;
        }

        File* open_file(string filename){
            this->fp = fopen(filename, "r");
            return this->fp;
        }

        void close_file(){
            fclose(this->fp);
            this->fp = NULL;
        }

};

