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
ID_default -> This is a very important define, as this is like the MAC of the device.This cannot be change through communication, only by hand, so change before uploading code.<br /> 


Other important variables are: <br />
thingsboard_topic_in -> Broker topic for sending messages <br />
thingsboard_topic_out -> Topic on which devices send info about their state <br />
thingsboard_topic_request -> A topic specifically designed for requesting info about device state <br />
thingsboard_topic_set -> Topic for changing group or serialNumber of a device


<br />
Valid messages with their respective answers in "lamps_out" topic (please change all instances of serialNumber, group, ID, 127.0.0.1 and topic names to those you edited in your .ino code!) <br />

```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_in" -m '{"relay_pin_state":true, "serialNumber":"SP-004"}'
{"serialNumber":"SP-004","ID":"BB-9","group":"AAAC","relay_pin":5,"dimming_pin":16,"dimming_pin_state":35,"relay_pin_state":true,"rssi":-83}
```

```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_in" -m '{"dimming_pin_state":240, "group":"AAAC"}'
{"serialNumber":"SP-004","ID":"BB-9","group":"AAAC","relay_pin":5,"dimming_pin":16,"dimming_pin_state":240,"relay_pin_state":true,"rssi":-81}
```


```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_set" -m '{"ID": "BB-9" , "group":"DD"}'
{"serialNumber":"SP-004","ID":"BB-9","group":"DD","relay_pin":5,"dimming_pin":16,"dimming_pin_state":2,"relay_pin_state":true,"rssi":31}
```

```
mosquitto_pub -h 127.0.0.1 -p 1883 -t "lamps_set" -m '{"ID": "BB-9" , "serialNumber":"AA8"}'
{"serialNumber":"AA8","ID":"BB-9","group":"AAAC","relay_pin":5,"dimming_pin":16,"dimming_pin_state":2,"relay_pin_state":true,"rssi":31}
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

* Added EEPROM storage with the functionality of setting group and serialNumber. These changes are preserved when the board is shutdown.  
Unfortunately, due to the use of flash memory, sometimes the board can misinterpret a given message. This can be fixed with a simple restart. In future version will add a memory card, which will be much more stable.
* An overall overhaul of the code is needed. For example, dividing it up into different parts and better comments can help future development.

