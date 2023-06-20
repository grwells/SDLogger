#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>

#include <MQTTMailer.hpp>
#include <TimeStamp.hpp>
#include <SDLogger.hpp>
#include <SDReader.hpp>

#include <string>


using namespace std;

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);



#define NETWORK "network_name_here"
#define PSSWD "password_here"
#define MQTT_BROKER_ADDR "broker.ip.addr.here"
#define MQTT_PORT 1883
#define DEBUG 1
#define WIFI_TIMEOUT 10000
const bool USB_DEBUG = DEBUG;

#define NUM_RECORDS 100
static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be in internal RAM

/**
 * @brief      Called when MQTT message is passed to the device by the broker
 *
 * @param      topic    The topic the message was published on
 * @param      message  The message in the MQTT packet
 * @param[in]  length   The length of the message
 */
void callback(char* topic, byte* message, unsigned int length){
    std::string msg_str;
    for(int i = 0; i < length; i++){
        msg_str += (char)message[i];
    }

    if(DEBUG) Serial.printf("[MQTT] Received \'%s\'|\'%s\'\n", topic, msg_str.c_str());
    // TODO: check if something needs to be done when receiving MQTT packets
}

void setup(){
    //ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );
    Serial.begin(9600);

    delay(5);

    WiFi.begin(NETWORK, PSSWD);
    //wifi_client.setInsecure();

    // default version print
    Serial.printf("Gator MAC address: %s\n", WiFi.macAddress().c_str());

    long int t0 = millis();
    long int time_elapsed = 0;
    while(WiFi.status() != WL_CONNECTED){
        time_elapsed = millis() - t0;
        delay(1000);
        Serial.print(".");

        if (time_elapsed > (WIFI_TIMEOUT)){
            Serial.println("\n[WARNING] WIFI_TIMEOUT");
            break;
            // collect data, but store locally
        }    
    }

    // can start time client w/o wifi
    //  but can't update

    if(DEBUG && WiFi.status() == WL_CONNECTED){
        Serial.println("\nCONNECTED");
        Serial.println();
        Serial.print("Gator connected @ ");
        Serial.println(WiFi.localIP());
        Serial.printf("Gator MAC address: %s\n", WiFi.macAddress().c_str());
    }

    // configue mqtt client connection
    mqtt_client.setBufferSize(34464);
    mqtt_client.setKeepAlive(120);
    mqtt_client.setServer(MQTT_BROKER_ADDR, MQTT_PORT);
    mqtt_client.setCallback(callback);

    // connect to MQTT
    if(mqtt_client.connect("aggregator")){
        if(USB_DEBUG) Serial.println("connected to MQTT client!");
    }else{
        if(USB_DEBUG) Serial.println("[WARNING] error with MQTT connection");
        delay(2000);
    }

    SDReader sdr = SDReader(); 

    // topic filter
    vector<string> tf;
    tf.push_back("kkm");

    Serial.println("====STARTING====");
    sdr.read_entry_range_from_files(TimeStamp(1672531199), TimeStamp(1702382400), tf, 120);
    Serial.println("====DONE====");
}

void loop(){

}
