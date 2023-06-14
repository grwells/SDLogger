#include "SDLoggerDataEntry.hpp"

/*
 * Convert member fields(timestamp, topic, message) to an MQTT message body
 *  and use the provided topic parameter as the new topic
 */
string SDLoggerDataEntry::to_mqtt_message(string topic){

    string msg_body = "{ \"timestamp\": \"" + this->timestamp->to_string() +
        "\", \"topic\": \"" + this->topic + "\"," +
        "\"body\": \"" + this->data + "\"}";

    MQTTMail msg(topic, msg_body);

    return msg.to_string();
}
