#include "SDReader.hpp"

bool SDReader::initialize_sd_card(){

    const auto ok = this->sd.begin(TT_CLK, TT_MISO, TT_MOSI, TT_SS, &SPI);

    if(!ok){
        Serial.println("[ERROR] failed to initialize sd card");
        return false;
    }
    
    Serial.println("[DEBUG] success initializing sd card");
    return true;
}

string SDReader::read_line(){return "";}

vector<std::string> read_entry_range(TimeStamp epoch, TimeStamp terminus, vector<string> topic_filter, string prefix="log"){
    vector<string> a;
    return a;
}

