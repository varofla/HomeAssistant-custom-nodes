#include <Arduino.h>
#include <SPI.h>

//----- MQTT -----//
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <string.h>

WiFiClient espClient; 
PubSubClient client(espClient);

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_id = "";
const char* mqtt_password = "";

//----- PIN -----//
#define PIN_SWITCH 2

bool last_status = !digitalRead(PIN_SWITCH); // 닫히면 HIGH, 열리면 LOW

//----- etc -----//
unsigned long nextTime = 0;

void MQTT_Discovery() {
  client.publish(
      "homeassistant/binary_sensor/Veranda_Curtain_Sensor/config",
      "{\"device_class\":\"door\",\"name\":\"VerandaCurtainSensor\",\"object_"
      "id\":\"Veranda_Curtain_Sensor\",\"unique_id\":\"Veranda_Curtain_"
      "Sensor\",\"device\":{\"identifiers\":\"Veranda_Curtain_Sensor\","
      "\"name\":\"VerandaCurtainSensor\",\"manufacturer\":\"VarOfLa\","
      "\"configuration_url\":\"https://blog.naver.com/dhksrl0508\"},\"state_"
      "topic\":\"homeassistant/binary_sensor/Veranda_Curtain_"
      "Sensor/state\"}",
      false);
}

void send_status(bool force_send=false) {
  if(force_send || last_status != !digitalRead(PIN_SWITCH)) {
    client.publish(
        "homeassistant/binary_sensor/Veranda_Curtain_Sensor/state",
        (last_status = !digitalRead(PIN_SWITCH)) ? "OFF" : "ON");  // 닫히면 OFF
  }
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "SensorNode-LTH-room"; // 클라이언트 ID
    if (client.connect(clientId.c_str(), mqtt_id, mqtt_password)) {
      MQTT_Discovery();
      send_status(true);
    } else {
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setBufferSize(1024);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (millis() >= nextTime) {
    MQTT_Discovery();
    nextTime = millis() + 300000; // 5분 간격
  }
  send_status();
}
