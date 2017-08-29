#include <ESP8266MQTTClient.h>

/**

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

WiFiClientSecure espClient;
WiFiUDP udp;
PubSubClient pubSubClient(espClient);
mDNSResolver::Resolver resolver(udp);

ESP8266MQTTClient mqttClient(&espClient, pubSubClient, &resolver);

void setup() {
  Serial.begin(115200);
  mqttClient.setup(onMessageReceived);
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

long lastMsg = 0;
char msg[50];
int value = 0;

void loop() {
  mqttClient.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttClient.sendMessage(msg);
  }
}
