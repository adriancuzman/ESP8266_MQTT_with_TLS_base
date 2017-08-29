# Description

The base/common code for sending and receiving information over MQTT on an ESP8266 with TLS encryption. This base can be used to quickly add a sensor or control a device over MQTT. Just add this library to have wifi with mDNS support and MQTT with TLS support. This is used to have smaller .ino files and not duplicate the wifi+mqtt setup code, just add sensor/device specific code.
Configuration is read from /config.json file on the ESP8266 file system. More info on this http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html

# Why?

When adding a new sensor or thing you have to add all the boilerplate code to setup wifi, mDNS, MQTT setup and TLS verification. In general all this setup is duplicated, so this library is a wrapper over this setup. In the end the adding a new sensor on the MQTT topics should focus on the sensor specific code and not the wifi+mqtt setup.

# Installation

Clone this code in to your [Arduino directory](https://www.arduino.cc/en/Guide/Libraries#toc5).

```bash
cd ~/Arduino
git clone git@github.com/adriancuzman/ESP8266MQTTClient
```

Restart your IDE. You can now include

```cpp
#include <ESP8266MQTTClient.h>
```

in your sketch.

# Usage

```cpp
#include <ESP8266MQTTClient.h>

WiFiClientSecure espClient;
WiFiUDP udp;
PubSubClient pubSubClient(espClient);
mDNSResolver::Resolver resolver(udp);

ESP8266MQTTClient mqttClient(&espClient, pubSubClient, &resolver);
```

In setup add
```cpp
  mqttClient.setup(onMessageReceived);
```
onMessageReceived - is a callback for receiving mqtt messages

```cpp
void onMessageReceived(char* topic, byte* payload, unsigned int length)
```

To send a message use:

```cpp
mqttClient.sendMessage(msg);
```

Also please look at the examples in the examples folder or examples in the IDE.

# Dependencies

ESP8266 for Arduino - https://github.com/esp8266/Arduino
mDNSResolver - https://github.com/madpilot/mDNSResolver


# Contributing

Issues and bugs can be raised on the [Issue tracker on GitHub](https://github.com/adriancuzman/ESP8266MQTTClient/issues)

For code and documentation fixes, clone the code, make the fix, write and run the tests, and submit a pull request.

Feature branches with lots of small commits (especially titled "oops", "fix typo", "forgot to add file", etc.) should be squashed before opening a pull request. At the same time, please refrain from putting multiple unrelated changes into a single pull request.
