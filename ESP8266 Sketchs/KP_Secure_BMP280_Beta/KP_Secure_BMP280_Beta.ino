#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <credentials.h>

//WIFI VARIABLE
const char* wifiSsid = mySSID;
const char* wifiPass = myPASSWORD;
uint8_t bssid[] = {0xD8, 0x74, 0x95, 0xE2, 0xD5, 0x8C};
int32_t ch = 11;
IPAddress ip(192, 168, 1, 50);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 1, 1);
IPAddress dns(1, 1, 1, 1);
static const char *fingerprint PROGMEM = "B6 D6 05 51 4D 7D 32 E3 79 64 55 CF 41 34 F6 AA F0 93 BA 61";
WiFiClientSecure espClient;

//MQTT VARIABLE
const char* mqtt_server = "192.168.1.100";
const char* topic = "iot/node/1";
PubSubClient client(espClient);

//SENSOR VARIABLE
Adafruit_BMP280 bmp;
float cal = 0.00433; // analog to voltage calibration value
String sensorTopic;
String sensorPayload;
String batteryTopic;
String batteryPayload;

//MQTT RECONNECT
void reconnect() {
  while (!client.connected()) {
    client.connect(topic);
    yield();
  }
}

void setup() {
//SENSOR INITIALISATION
  bmp.begin(0x76);              // Initialisation with the alternative I2C address (0x76)
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X1,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X1,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X4,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
                  
//GATHER SENSOR & BATTERY DATA
  sensorTopic += topic;
  sensorTopic += "/sensor_data";
  sensorPayload += "{\"pressure\":";
  sensorPayload += bmp.readPressure()/99.99F;
  sensorPayload += ",\"altitude\":";
  sensorPayload += bmp.readAltitude(1013.25)*1.055F;
  sensorPayload += "}";
  batteryTopic += topic;
  batteryTopic += "/battery_level";
  batteryPayload += "{\"battery_level\":";
  batteryPayload += analogRead(A0)*cal;
  batteryPayload += ",\"raw_analog\":";
  batteryPayload += analogRead(A0);
  batteryPayload += "}";
  
//WIFI INITIALISATION
  WiFi.begin(wifiSsid, wifiPass, ch, bssid);
  WiFi.config(ip, dns, gateway, subnet); 
  if (WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.reconnect();
  }

//MQTT CONNECTION
  espClient.setFingerprint(fingerprint);
  client.setServer(mqtt_server, 8883);
  if (!client.connected()) {
    reconnect();
  }
  client.publish(((char*) sensorTopic.c_str()), ((char*) sensorPayload.c_str()));
  client.publish(((char*) batteryTopic.c_str()), ((char*) batteryPayload.c_str()));
  delay(50);
  
//DEEPSLEEP
  ESP.deepSleep(1000000*60);
}
void loop() {}
