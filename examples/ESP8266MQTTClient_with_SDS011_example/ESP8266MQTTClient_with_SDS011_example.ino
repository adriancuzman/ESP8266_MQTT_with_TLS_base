#include <ESP8266MQTTClient.h>
#include <SDS011.h>
/**
  Example for ESP8266MQTTClient with the Air Quality SDS011 sensor

  The base/common code for sending and receiving information over MQTT on an ESP8266 with TLS encryption.
  This base can be used to quickly add a sensor or control a device over MQTT.
  Just add this library to have wifi with mDNS support and MQTT with TLS support.
  This is used to have smaller .ino files and not duplicate the wifi+mqtt setup code, just add sensor/device specific code.

  Configuration details will be loaded from file /config.json

  Example of a configuration file:
  {
  "wifi_ssid":"network_name",
  "wifi_password":"wifi_password",
  "mqtt_server":"hostname/url/ip/rasberrypi.local",
  "mqtt_server_port": 1883,
  "mqtt_user_name":"mqtt_user",
  "mqtt_user_password":"mqtt_user_password",
  "mqtt_client_name":"a client name, also used for mDNS ",
  "mqtt_input_topic":"default_input_topic_name",
  "mqtt_output_topic":"default_output_topic_name",
  "mqtt_server_fingerprint":"SHA1 fingerprint of the TLS certificate"
  }
*/

// clients used that need to be instatiated outside
WiFiClientSecure espClient;
WiFiUDP udp;
PubSubClient pubSubClient(espClient);

/** mDNS resolver is used to resolve local hosts,
  for example if your raspberrypi localhost is "rasberrypi.local"
  The library can be found on github - https://github.com/madpilot/mDNSResolver
*/
mDNSResolver::Resolver resolver(udp);

ESP8266MQTTClient mqttClient(&espClient, pubSubClient, &resolver);

//topic for pm2.5 particles
const char* pm25_topic = "sds11_01_pm10";

//topic for pm10 particles
const char* pm10_topic = "sds11_01_pm25";

// connect D1 to SDS011 tx pin
const int SDS011_tx_pin = D1;

// connect D2 to SDS011 rx pin
const int SDS011_rx_pin = D2;

// in seconds
int sleepTime = 600;
int wormupTime = 30;

unsigned long timeNow = 0;

unsigned long timeLast = 0;

unsigned long seconds = 0;

String message;

// sds sleep status
int sds_sleeping = 1;



int nrOfSamplesToRead = 5;
int maxReedAttempts = 100;

float p10, p25;
int error;

SDS011 my_sds;

void setup() {
  Serial.begin(115200);
  mqttClient.setup(onMessageReceived);
  setup_sds();
}

void loop() {
  mqttClient.loop();
  sds_loop();
}

void setup_sds() {
  Serial.println("---- sds setup --- ");
  my_sds.begin(SDS011_tx_pin, SDS011_rx_pin);
  my_sds.sleep();
  sds_sleeping = 1;
}

void sds_loop(){
  timeNow = millis() / 1000; // the number of milliseconds that have passed since boot

  seconds = timeNow - timeLast;

  if (seconds > sleepTime & sds_sleeping == 1) {
    Serial.println("Wakeup & worming up..");
    my_sds.wakeup();
    sds_sleeping = 0;
  }

  if (seconds > (sleepTime + wormupTime)) {
    readData();
    Serial.println("Sleep");
    my_sds.sleep();

    timeLast = timeNow;
    sds_sleeping = 1;
  }
}

void readData() {
  Serial.println("---- reading data started --- ");
  int readCount = 0;
  int readAttempts = 0;
  float p25Agregator = 0;
  float p10Agregator = 0;
  float p25Temp = 0;
  float p10Temp = 0;
  do {
    error = my_sds.read(&p25Temp, &p10Temp);
    readAttempts++;
    if (! error) {
      p25Agregator += p25Temp;
      p10Agregator += p10Temp;
      readCount++;
      Serial.println("---- Reading count: " + String(readCount) + "-----");
      Serial.println("P2.5: " + String(p25Temp));
      Serial.println("P10:  " + String(p10Temp));
    }
    delay(100);
  }
  while (readCount < nrOfSamplesToRead && readAttempts < maxReedAttempts);
  p25 = p25Agregator / readCount;
  p10 = p10Agregator / readCount;

  message += "---- Average readings at " + String(millis() / 1000) + "seconds from boot-----<br>";
  message += "P2.5: " + String(p25) + "<br>";
  message += "P10:  " + String(p10) + "<br>";

  Serial.println("---- Average readings:  -----");
  Serial.println("P2.5: " + String(p25));
  Serial.println("P10:  " + String(p10));

  mqttClient.sendMessage(pm25_topic, String(p25).c_str());
  mqttClient.sendMessage(pm10_topic, String(p10).c_str());
}

long lastMsg = 0;
char msg[50];
int value = 0;

void onMessageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

