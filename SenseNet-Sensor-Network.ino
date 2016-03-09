
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include "DHT.h"
// ------------- WiFi Settings -------------

const char* ssid = "SenseNet";
const char* password = "sensenet192";

// ------------- MQTT Settings -------------

char server[] = "mqtt1.sensenet.co.uk";
char topic_root[] = "sensenet/esp8266/1/";
char clientId[] = "prototype1";

// ------------- Pin Settings -------------

#define DHT_PIN 2
#define DHT_POWER_PIN 15
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// ------------- General Settings -------------

unsigned long sleep_length = 120000000;

#define MQTT_KEEPALIVE 300000

// End Settings

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wifi_client;
PubSubClient client(server, 1883, callback, wifi_client);
DHT dht(DHT_PIN, DHTTYPE, 11);
int fail_count=0;
static char ssid2[20];


void setup() {
  // setup wifi
  WiFi.SSID().toCharArray(ssid2,20);
  if (strcmp (ssid2, ssid) != 0) {
    WiFi.begin(ssid, password);
  }
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
  }
  // setup dht
  pinMode(DHT_POWER_PIN, OUTPUT);
  digitalWrite(DHT_POWER_PIN, HIGH);
  delay(100);
  dht.begin();
}

void sleep(){
  digitalWrite(DHT_POWER_PIN, LOW);
  ESP.deepSleep(sleep_length);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!!!client.connected()) {
    while (!!!client.connect(clientId)) {
      delay(500);
    }
    Serial.println();
  }

  //read some values

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  while (isnan(t)){
    if (fail_count >= 5){
      client.publish(topic_root, "Too many failures, going back to sleep");
      fail_count=0;
      sleep();
    }
    fail_count += 1;
    delay(2000);
    h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();
    client.publish(topic_root, "Reading failed, retrying");
  }
  //convert these to strings
  static char temp[9];
  static char humidity[9];
  dtostrf(t,5,2,temp);
  dtostrf(h,5,2,humidity);

  String temp_topic(topic_root);
  temp_topic = temp_topic + "temperature";
  client.publish(temp_topic.c_str(), temp); 

  String humidity_topic(topic_root);
  humidity_topic = humidity_topic + "humidity";
  client.publish(humidity_topic.c_str(), humidity);
  
  //go to sleep
  sleep();
}



void callback(char* topic, byte* payload, unsigned int length) {
 
}
