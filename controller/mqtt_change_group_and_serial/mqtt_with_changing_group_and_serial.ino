#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "FS.h"

//Device specific telemtry+attributes used in generation of JSON
//in later patch will be configured to be taken directly from the device
#define TOKEN  "2USDtz24ztIXHqlK7QRw"
#define serialNumber_UNIQ "SP-004"
#define group_UNIQ "AAAC"
#define dimming_pin_state_default 0
#define relay_pin_state_default true
#define ID_default "BB-9" // this cannot be changed, while group and serial can be changed later on





// Connect to the WiFi
const char* ssid = "****";
const char* password = "****";
const char* mqtt_server = "****";
const long mqtt_port = 1883;
//mqtt pub sub conf
String thingsboard_topic_in = "lamps_in";
String thingsboard_topic_out = "lamps_out";
String thingsboard_topic_request ="lamps_request";
String thingsboard_topic_set= "lamps_set";

//messages setup
const String relay_on_message = "switchon";
const String relay_off_message = "switchoff";
const String get_pin_message = "getPin";

//callback header needs to be defined before PubSubClient in order to publish msg withing callback
void callback(char* topic, byte* payload, unsigned int length);


//SPIFFS file config
const String file_path="/variables.json";

WiFiClient espClient;
PubSubClient client(mqtt_server, mqtt_port, callback, espClient);







////////////////// the json properties of device
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class json_device {
  //DEFINE DEVICE SPECIFIC DATA
  private:
    String return_serialNumber();
    String ID = ID_default;
    String return_group();
    long relay_pin = 5;
    long dimming_pin = 16;
    //pin checking funcs
    bool relay_pin_state();
    long dimming_pin_state();
  public:
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& msg = jsonBuffer.createObject();
    json_device(); // this will be the initial configuration
    //json string
    String generateMessage();

    bool checkID(JsonObject& message);
    bool checkGroupAndSerial(JsonObject&);
    
    void process_command_msg(JsonObject& );
    String process_request_msg(JsonObject&);
    void process_set_msg(JsonObject&);

    void WriteToFile();
    void ReadFromFile();
};






///////////////////////json_device methods


///////////////////////////////////////////////////////////////////////////////////////// Read device group serialNumber and pin states from variables.json



String json_device::return_group() {
  File f = SPIFFS.open(file_path, "r");
    if (!f) {
      Serial.println("file open failed");
  }else{  
    Serial.println("Taking group from file");
  // parse the config file to a json
    StaticJsonBuffer<200> jsonBuffer_file;
    JsonObject& json_from_file= jsonBuffer_file.parseObject(f);
       String key;
  for (JsonObject::iterator it = json_from_file.begin(); it != json_from_file.end(); ++it) {
    key = it->key;
    if (key == "group") {
    f.close();
    return json_from_file["group"];
    }
  
}
}
Serial.println("Couldn't find group from file, returning default state");
f.close();
return group_UNIQ;
}


// returns serialNumber pin state from file
String json_device::return_serialNumber() {
  File f = SPIFFS.open(file_path, "r");
    if (!f) {
      Serial.println("file open failed");
  }else{  
    Serial.println("Taking serialNumber from file");
  // parse the config file to a json
    StaticJsonBuffer<200> jsonBuffer_file;
    JsonObject& json_from_file= jsonBuffer_file.parseObject(f);
       String key;
  for (JsonObject::iterator it = json_from_file.begin(); it != json_from_file.end(); ++it) {
    key = it->key;
    if (key == "serialNumber") {
     f.close();
    return json_from_file["serialNumber"];
    }
  
}
}
Serial.println("Couldn't find serialNumber from file, returning default state");
f.close();
return serialNumber_UNIQ;
}


// returns relay pin state from file
bool json_device::relay_pin_state() {
  File f = SPIFFS.open(file_path, "r");
    if (!f) {
      Serial.println("file open failed");
  }else{  
    Serial.println("Taking relay_pin_state from file");
  // parse the config file to a json
    StaticJsonBuffer<200> jsonBuffer_file;
    JsonObject& json_from_file= jsonBuffer_file.parseObject(f);
       String key;
  for (JsonObject::iterator it = json_from_file.begin(); it != json_from_file.end(); ++it) {
    key = it->key;
    if (key == "relay_pin_state") {
    f.close();
    return json_from_file["relay_pin_state"];
    }
  
}
}
Serial.println("Couldn't find relay_pin_state from file, returning default state");
f.close();
return relay_pin_state_default;
}


// returns dimming pin state from file
long json_device::dimming_pin_state() {
  File f = SPIFFS.open(file_path, "r");
    if (!f) {
      Serial.println("file open failed");
  }else{  
    Serial.println("Taking dimming_pin_state from file");
  // parse the config file to a json
    StaticJsonBuffer<200> jsonBuffer_file;
    JsonObject& json_from_file= jsonBuffer_file.parseObject(f);
       String key;
  for (JsonObject::iterator it = json_from_file.begin(); it != json_from_file.end(); ++it) {
    key = it->key;
    if (key == "dimming_pin_state") {
    f.close();
    return json_from_file["dimming_pin_state"];
    }
  
}
}
Serial.println("Couldn't find dimming_pin_state from file, returning default state");
f.close();
return dimming_pin_state_default;
}


///////////////////////////////////////////////////////////////////////////////////////// EOF file reading



json_device::json_device() {
  msg["serialNumber"] = return_serialNumber();
  msg["ID"] = ID;
  msg["group"] = return_group();
  msg["relay_pin"] = relay_pin;
  msg["dimming_pin"] = dimming_pin;
  msg["dimming_pin_state"] = dimming_pin_state();
  msg["relay_pin_state"] = relay_pin_state();
  msg["rssi"] = WiFi.RSSI();
}

///////////////////////////////////////////////////////////////////////////////////////// Functions for checking if messages are intended for the current device

bool json_device::checkGroupAndSerial(JsonObject& message){
    bool groupcheck = false, serialcheck = false;
    String key;
  for (JsonObject::iterator it = message.begin(); it != message.end(); ++it) {
    key = it->key;
    if (key == "group" && it->value==return_group()) {
      groupcheck = true;
    }
    if (key == "serialNumber" && it->value==return_serialNumber()) {
      serialcheck = true;
    }
  }
  if (groupcheck == 1) {
    Serial.println("group checks out");
  } else {
    Serial.println("invalid group");
  }
  if (serialcheck == 1) {
    Serial.println("serial checks out");
  } else {
    Serial.println("invalid serial");
  }
  if (groupcheck==1 || serialcheck==1){return true;}
  else{return false;}
  }

bool json_device::checkID(JsonObject& message){
    bool IDcheck = false;
    String key;
  for (JsonObject::iterator it = message.begin(); it != message.end(); ++it) {
    key = it->key;
    if (key == "ID" && it->value==msg[key]) {
      IDcheck = true;
    }
  }
  if (IDcheck == true) {
    Serial.println("ID checks out");
  } else {
    Serial.println("invalid ID");
  }
  return IDcheck;
  }
  
///////////////////////////////////////////////////////////////////////////////////////// EOF check functions


String json_device::generateMessage() {
  char payload[200];
  msg.printTo(payload, sizeof(payload));
  String strPayload = String(payload);

  return strPayload;
}


///////////////////////////////////////////////////////////////////////////////////////// Message processing functions

String json_device::process_request_msg(JsonObject& message){
  char payload[200];
  String key;
  StaticJsonBuffer<200> jsonBuffer_answer;   
   JsonObject& answer=jsonBuffer_answer.createObject();
  
  File f = SPIFFS.open(file_path, "r");
    if (!f) {
      Serial.println("file open failed");
  } 
  StaticJsonBuffer<200> jsonBuffer_read_file;
  JsonObject& read_from_file=jsonBuffer_read_file.parseObject(f);

  for (JsonObject::iterator it = message.begin(); it != message.end() ; ++it) {
    if (it == message.begin()) {
      Serial.println("Request message processing entered");
    }
    key=it->key;
    if (key != "group" && key!="serialNumber"){
     Serial.print("key = ");
     Serial.println(key);
     answer[key]=read_from_file[key];
      }
    }
  
  answer.printTo(payload, sizeof(payload));
  String strAnswer = String(payload);
  Serial.print("answer string: ");
  Serial.println(strAnswer);
  f.close();
  return strAnswer;

}


void json_device::process_command_msg(JsonObject& message) {
  
  for (JsonObject::iterator it = message.begin(); it != message.end()   ; ++it) {
    //here we will describe the message cases:
    if (it == message.begin()) {
      Serial.println("Command message processing entered");
    }
    String key = it->key;
    Serial.print("message received for followin attribute: "); Serial.print(key); Serial.print("="); Serial.println(it->value.asString());
    if (key == "relay_pin_state" && message[key].is<bool>()) {
      Serial.println("Setting up relay");
      msg["relay_pin_state"]= it->value;
      digitalWrite(msg["relay_pin"], it->value ? HIGH : LOW);
    }
    if (key == "dimming_pin_state" && message[key].is<long>() && it->value <= 255 && it->value >= 0) {
      // analogWriteFreq(5150);
      Serial.println("Setting up PWM");
      long dimming_level = 255 - long(it->value);
      dimming_level = map(dimming_level, 0, 255, 0 , 1023);
      analogWrite(msg["dimming_pin"], dimming_level);
      msg["dimming_pin_state"]=it->value;
      // msg["relay_pin_state"]= true;
      if (it->value == 0) {
        Serial.println("received dimming level 0. Turning off relay....");
        digitalWrite(msg["relay_pin"], LOW);
        msg["relay_pin_state"]=false;
      }
    }
  }
  
   Serial.println("END of command message");
   msg["group"]= return_group();
   msg["serialNumber"]= return_serialNumber();
   Serial.print("group = ");Serial.println(msg["group"].asString());
   Serial.print("serialNumber = ");Serial.println(msg["serialNumber"].asString());
}



void json_device::process_set_msg(JsonObject& message) {
 
  for (JsonObject::iterator it = message.begin(); it != message.end()   ; ++it) {
    //here we will describe the message cases:
    if (it == message.begin()) {
      Serial.println("Set message processing entered");
    }
    String key = it->key;
    Serial.print("message received for followin attribute: "); Serial.print(key); Serial.print("="); Serial.println(it->value.asString());
    
    if (key == "group" && message[key].is<char*>()) {
      Serial.println("Setting up group= ");
      Serial.println(it->value.asString());
      msg["group"]= it->value.asString();
    }
    if (key == "serialNumber" && message[key].is<char*>() ) {
        Serial.print("Setting up serialNumber = ");
        Serial.println(it->value.asString());
        msg["serialNumber"]=it->value.asString();
    }
   }
   //some issus with relay and dimming states when setting fgroup or serial
 msg["relay_pin_state"]= relay_pin_state();
 msg["dimming_pin_state"]= dimming_pin_state();
}


///////////////////////////////////////////////////////////////////////////////////////// EOF message processing functions


///////////////////////////////////////////////////////////////////////////////////////// Functions for editing the variables.json file



void json_device::WriteToFile(){
   
   StaticJsonBuffer<200> jsonBuffer_file;
  JsonObject& file_msg = jsonBuffer_file.createObject();
  file_msg["serialNumber"]=msg["serialNumber"];
  file_msg["group"]=msg["group"];
  file_msg["relay_pin_state"]=msg["relay_pin_state"];
  file_msg["dimming_pin_state"]=msg["dimming_pin_state"];  
  File f = SPIFFS.open(file_path, "w");
  if (!f) {
      Serial.println("file open failed");
  }
  Serial.println("====== Writing to SPIFFS file =========");

  char payload[200];
  file_msg.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.println(strPayload);
  f.println(strPayload);
  f.close();
}

void json_device::ReadFromFile(){
  File f = SPIFFS.open(file_path, "r");
    if (!f) {
      Serial.println("file open failed");
  }  Serial.println("====== Reading from SPIFFS file =======");
  // read 10 strings from file
    StaticJsonBuffer<200> jsonBuffer_file;
    JsonObject& json_from_file= jsonBuffer_file.parseObject(f);
     char payload[200];
  json_from_file.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.println(strPayload); 
  f.close();
  
  }

///////////////////////////////////////////////////////////////////////////////////////// EOF functions editing variables.json




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Create_file(){
  if (!SPIFFS.exists(file_path)){
  File f = SPIFFS.open(file_path, "w");
  if (!f) {
    Serial.println("file creation failed");
  }
     f.close();
  }

  }

void setup()
{
  json_device ESP;
  SPIFFS.begin();
  Create_file();
  Serial.begin(9600);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(ESP.msg["relay_pin"], OUTPUT);
  pinMode(ESP.msg["dimming_pin"], OUTPUT);
}


json_device current_device;

//mqtt message receiving fnc
void callback(char* topic, byte* payload, unsigned int length) {

  char message[length];
  String mqtt_topic = topic;
  
  strncpy(message, (char*)payload, length);
  StaticJsonBuffer<200> jsonBuffer_received;
  JsonObject& received_json = jsonBuffer_received.parseObject(message);
  if (!received_json.success()) {
    Serial.println("parseObject failed");
  } else {
    Serial.println("parseObject succeeded!");
  }
 
if(current_device.checkGroupAndSerial(received_json)){
 if(mqtt_topic==thingsboard_topic_in){ 
      current_device.process_command_msg(received_json);
      current_device.WriteToFile();
      current_device.ReadFromFile();
      client.publish(thingsboard_topic_out.c_str(), current_device.generateMessage().c_str());

}
  if(mqtt_topic==thingsboard_topic_request){
    String answer=current_device.process_request_msg(received_json);
     client.publish(thingsboard_topic_out.c_str(), answer.c_str());
  }
 }
if(current_device.checkID(received_json)){ 
  if(mqtt_topic==thingsboard_topic_set){    
    current_device.process_set_msg(received_json);
    current_device.WriteToFile();
    current_device.ReadFromFile();
    client.publish(thingsboard_topic_out.c_str(), current_device.generateMessage().c_str());
  
  }
}

}








void WiFiInit() {

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(500);
  }
  Serial.print("Connection status:");
  Serial.println(WiFi.status());
  Serial.println(WiFi.localIP());
}


void MQTTInit() {
 
  // Loop until we're connected
  while (!client.connected() ) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(serialNumber_UNIQ, TOKEN, NULL)) {
      Serial.println("connected");

      client.subscribe(thingsboard_topic_in.c_str());
      client.subscribe(thingsboard_topic_request.c_str());
      client.subscribe(thingsboard_topic_set.c_str());

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}







void loop()
{
  //if we dont have wifi connection try to intialise wifi and mqtt
  if (WiFi.status() != WL_CONNECTED) {
    
    WiFiInit();
    MQTTInit();
      
  }


  client.loop();
}
