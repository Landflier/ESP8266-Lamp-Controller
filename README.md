# ESP8266-Lamp-Controller


The c++ code for receiving message from an UI or just mqtt messages, processing them and then turning on/off and dimming a lamp.
The current code works with a Wi-Fi connection, but can be easily ported to ethernet.


## Deployment
### Connection

To deploy locally, please edit the following variables :<br />
ssid -> name of your Wi-Fi <br />
password -> Password of Wi-Fi<br />
mqtt_server -> The address, where the mqtt-broker is running<br />
mqtt_port -> The port on which mqtt-broker is running, though it usually is 1883<br />

### Messaging
The code has an added functionality for a device to indentify when a message is addressed to it. If you dont want to use 
this functionality, you can remove the checkGroupAndSerial() condition in the void callback() function. <br />

If you do want to use this functionality however, edit the following constants: <br />
serialNumber_UNIQ -> The serial number of a device <br />
group_UNIQ -> The group of a device<br />

Other important variables are: <br />
thingsboard_topic_in -> Broker topic for sending messages <br />
thingsboard_topic_out -> Topic on which devices send info about their state <br />
thingsboard_topic_request -> A topic specifically designed for requesting info about device state <br />


<br />
Valid messages with their respective answers in "lamps_out" topic (please change all instances of serialNumber, group, 127.0.0.1 and topic names to those you edited in your .ino code!) <br />

```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_in" -m '{"relay_pin_state":true, "serialNumber":"SP-004"}'
{"serialNumber":"SP-004","ID":"BB-8","group":"AAAC","relay_pin":5,"dimming_pin":16,"dimming_pin_state":35,"relay_pin_state":true,"rssi":-83}
```

```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_in" -m '{"dimming_pin_state":240, "group":"AAAC"}'
{"serialNumber":"SP-004","ID":"BB-8","group":"AAAC","relay_pin":5,"dimming_pin":16,"dimming_pin_state":240,"relay_pin_state":true,"rssi":-81}
```


```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_in" -m '{"relay_pin_state":false, "dimming_pin_state" :140, "serialNumber":"SP-004"}'
{"serialNumber":"SP-004","ID":"BB-8","group":"AAAC","relay_pin":5,"dimming_pin":16,"dimming_pin_state":140,"relay_pin_state":false,"rssi":-80}
```


```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_request" -m '{"relay_pin_state":, "serialNumber":"SP-004"}'
{"relay_pin_state":true}
```

## Built With

* [PubSubClient](https://pubsubclient.knolleary.net/) - MQTT for Arduino
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - Use of JSON type messages in the mqtt communication
* [FileSystem](http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html)- The EEPROM storage on the ESP8266
* [ESP8266](https://www.olimex.com/Products/IoT/ESP8266-EVB-BAT/open-source-hardware) - ESP-8266 board documentation


## Authors

Vasil Yordanov<br />
For further information contact vasil.r.yordanov@gmail.com

## License

This project is licensed under the GPLv3 License - see the [GPLv3](https://www.gnu.org/licenses/quick-guide-gplv3.en.html) site for details

## Remarks

* EEPROM storage was added in the first place with the idea that one could change the serialNumber and group of a device later on after boot. 
This feature is not yet fully added, therefore serial and group are hardcoded. Implementating this can be done in the WriteToFile() funciton <br />
However this library helps with reading the dimming state, since AnalogRead() does not work well with the ESP8266 Board I am using.
* An overall overhaul of the code is needed. For example dividing it up into different parts, better comments, consistency of variable names.

