#include "SDLogger.hpp"

SDLogger::SDLogger(std::string filename){
    this->filename = filename + filetype;
    
    this->initialize_sd_card();
}


/**
 * Constructs a SDLogger instance with file name by appending all the provided parameters separated by single
 * characters (`_`, `-`, `.`). 
 *
 * ```
 * <prefix>_<month>-<day>-<year>.<filetype>
 * ```
 *
 * @param[in] prefix The first part of the filename, identifies purpose of file.
 * @param[in] month The month the file corresponds to, integer on [1,12].
 * @param[in] day The day, an integer [1, 31].
 * @param[in] year The year, ex `2023`.
 * @param[in] filetype The file/data type, ex `csv`
 */
SDLogger::SDLogger(std::string prefix, int month, int day, int year, std::string filetype){
    this->filename = prefix + 
        "_" + std::to_string(month) + 
        "-" + std::to_string(day) +
        "-" + std::to_string(year) + 
        filetype;
}

/**
 * @brief Call before using SD card interface. Initializes connection to SD card.
 *
 * @returns `true` if successfully initialized.
 */
bool SDLogger::initialize_sd_card(){

    const auto ok = this->sd.begin(TT_CLK, TT_MISO, TT_MOSI, TT_SS, &SPI);

    if(!ok){
        Serial.println("[ERROR] failed to initialize sd card");
        return false;
    }
    
    Serial.println("[DEBUG] success initializing sd card logger");
    Serial.print("[DEBUG] bytes free =  ");
    double perc = ESP.getFreeHeap();
    Serial.println(perc);
    return true;
}

/**
 * Stores a file name in this object for repeated access. Use if planning
 * to use this object for operating on one file only.
 *
 * @param[in] fn The file name to open/close. 
 */
void SDLogger::set_filename(std::string fn){
    this->filename = fn;
}

/**
 * Creates a filename by concatenating the parameters to this method, separated by `_`, `-`, `.`. Unlike 
 * `set_filename(string)` this method allows the user to change the date, prefix
 * and filetype (file extension) associated with the file.
 *
 * ```
 * <prefix>_<month>-<day>-<year>.<filetype>
 *```
 * @param[in] prefix The first string in the filename, ex `log`
 * @param[in] month The month this file was created, ex `09` or `september`
 * @param[in] day The day this file was created, ex `12`
 * @param[in] year The year this file was created, ex `2023`
 * @param[in] filetype The filetype/extension of the file, ex `.csv` or `.txt`
 */
void SDLogger::set_filename(std::string prefix, int month, int day, int year, std::string filetype){
    this->filename = prefix + 
        "_" + std::to_string(month) + 
        "-" + std::to_string(day) +
        "-" + std::to_string(year) + 
        filetype;
}

/**
 * Opens a file in one of the two access modes, read only, or read/write.
 *
 * @param[in] mode `r`, `w`, etc
 *
 * @returns An open file object. If it fails, the error is printed and it should stop execution.
 */
File SDLogger::open_file(const char* mode){
    try{
        Serial.printf("\t-> trying to open file \'%s\' in mode -> %s\n", this->filename.c_str(), mode);
        return this->sd.open(this->filename.c_str(), mode);

    }catch(const std::exception& e){
        Serial.println("print error in open");
        Serial.println(e.what());
     
    }catch(...){
        Serial.println("ERROR opening file");
    }

}


/**
 * Closes the connection to the SD card. This is effectively shutting down 
 * the connection through the SPI interface.
 */
void SDLogger::close_card(){
    this->sd.end();
}

/**
 * Write a line to a file, this replaces whatever is currently in the file so use
 * with caution. File is closed at end of operation.
 *
 * Note that a newline character is appended to the end of the input parameter.
 *
 * @param[in] line The string data which is written as a "line". 
 */
void SDLogger::write_line(std::string line){
    File f = this->open_file(FILE_WRITE);

    f.write('\n');

    const char* buf = line.c_str();
    f.write((uint8_t*)buf, line.length());
    f.close();
}

/**
 * Appends a line of text to the end of a file's contents. Note that 
 * a newline character is appended to the end of the input string to 
 * create a "line" and that the file is closed after the operation.
 *
 * @param[in] line The text to append to the end of the file.
 */
void SDLogger::append_line(std::string line){
    File f = this->open_file(FILE_APPEND);

    f.write('\n');

    const char* buf = line.c_str();
    f.write((uint8_t*)buf, line.length());
    f.close();
}


/**
 * Logs an mqtt topic message to a file. This means that the logged data is 
 * timestamped, has a topic, and a message. The line is appended to the file
 * using `append_line(string)` from this class.
 *
 * @param[in] time The timestamp as a string formatted `<date_string>T<time_string>`.
 * @param[in] mqtt_topic The topic string as a `<base>/<subtopic>/...` formatted string.
 * @param[in] mqtt_message A string, often JSON object string but not always.
 */
void SDLogger::log_absolute_mqtt(std::string time, std::string mqtt_topic, std::string mqtt_message){
    std::string line = time + separator + 
        mqtt_topic + separator + 
        mqtt_message + separator;

    this->append_line(line);
}

/**
 * Logs an mqtt topic message to a file. This means that the logged data is 
 * timestamped, has a topic, and a message. The line is appended to the file
 * using `log_absolute_mqtt(string)` from this class. Essentially a wrapper 
 * which adds the `offset` parameter to provide a relative time estimate in
 * minutes from the previous known time. 
 *
 * @param[in] time The timestamp as a string formatted `<date_string>T<time_string>`.
 * @param[in] offset The relative offset in minutes, probably calculated from the WDT module reset frequency.
 * @param[in] mqtt_topic The topic string as a `<base>/<subtopic>/...` formatted string.
 * @param[in] mqtt_message A string, often JSON object string but not always.
 */
void SDLogger::log_relative_mqtt(std::string time, int offset, std::string mqtt_topic, std::string mqtt_message){
    time = time + "+" + std::to_string(offset);

    this->log_absolute_mqtt(time, mqtt_topic, mqtt_message);
}

/**
 * Used to initialize a CSV file by writing the list of comma 
 * separated fields to the first line of the file. In this case
 * the fields are provided as a vector and are written using
 * `write_line(string)` so the values overwrite all other data
 * in the file.
 *
 * @param[in] fields A vector list of strings which represent data columns/fields in the CSV
 * file.
 */
void SDLogger::write_header(std::vector<std::string> fields){
    std::string header = "";
    for(std::string i : fields){
        header += i + separator;
    }

    this->write_line(header);
}
