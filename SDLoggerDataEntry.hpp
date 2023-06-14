#include <string>
#include <vector>
#include "../TimeStamp/TimeStamp.hpp"
#include "../MQTTMailer/MQTTMailer.hpp"

using namespace std;

/*
 * Class defining the syntax for a data entry by the SDLogger. Used for storing and retrieving data, as well as formatting and 
 * performing basic operations/conversions.
 *
 */
class SDLoggerDataEntry {

    public:

        TimeStamp* timestamp;    // timestamp data was recorded at
        string topic;           // mqtt topic/type of data recorded here
        string data;            // the data string
                                //

        SDLoggerDataEntry(){;}

        SDLoggerDataEntry(TimeStamp ts, string topic, string data){
            this->timestamp = &ts;
            this->topic = topic;
            this->data = data;
        }

        string to_mqtt_message(string topic);   // convert fields to an mqtt message 
                                                //
        
        string to_string(){ return this->timestamp->to_string() + ", " + topic + ", " + data; }

};
    

