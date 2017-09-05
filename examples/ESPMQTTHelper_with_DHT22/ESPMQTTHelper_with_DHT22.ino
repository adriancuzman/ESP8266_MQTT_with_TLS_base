#include <ESPMQTTHelper.h>
#include "DHT.h"

/**
  Example for ESPMQTTHelper with DHT22 sensor

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

#define DHTPIN D3     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

char* temperature_topic = "DHT22_01_temperature";
char* humidity_topic = "DHT22_01_humidity";
char* heat_index_topic = "DHT22_01_heat_index";

// clients used that need to be instatiated outside
WiFiClientSecure espClient;
WiFiUDP udp;
PubSubClient pubSubClient(espClient);

/** mDNS resolver is used to resolve local hosts,
  for example if your raspberrypi localhost is "rasberrypi.local"
  The library can be found on github - https://github.com/madpilot/mDNSResolver
*/
mDNSResolver::Resolver resolver(udp);

ESPMQTTHelper mqttHelper(&espClient, pubSubClient, &resolver);

long lastMsg;

void setup() {
  Serial.begin(115200);
  mqttHelper.setup(onMessageReceived);
  lastMsg =  millis();
}

void loop() {
  mqttHelper.loop();

  long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    readDHTValues();
  }
}

void readDHTValues() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
 
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.println();

  mqttHelper.sendMessage(temperature_topic, String(t).c_str());

  mqttHelper.sendMessage(humidity_topic, String(h).c_str());

  mqttHelper.sendMessage(heat_index_topic, String(hic).c_str());
}

void onMessageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
