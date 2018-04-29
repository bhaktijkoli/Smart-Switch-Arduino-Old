#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

void setAll(int value);
void updateStates();
String formatStatus();

int TOTAL = 4;
int DELAY = 2000;

ESP8266WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81);

int buttons[] = {D5, 56, D7, D8};
int leds[] = {D1, D2, D3, D4};
int states[] = {LOW, LOW, LOW, LOW};

void setup() {
  for(int i=0;i<TOTAL;i++) {
    pinMode(buttons[i], INPUT);
  }
  for(int i=0;i<TOTAL;i++) {
    pinMode(leds[i], OUTPUT);
  }
  setAll(LOW);
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  Serial.println("Started");
  WiFiMulti.addAP("Bhk", "12345678");
  Serial.print("Connecting");
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  while(!MDNS.begin("touchswitch")) {
    delay(100);
  }
  MDNS.addService("ws", "tcp", 81);
  Serial.println("DNS Started");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.printf("Web Socket Server started\n");
}

void loop() {
  webSocket.loop();
  updateStates();
  return;

  if(digitalRead(buttons[0]) == HIGH && digitalRead(buttons[1]) == HIGH) {
    setAll(HIGH);
    Serial.printf("All Off\n");
    webSocket.broadcastTXT(formatStatus().c_str());
    delay(DELAY);
  }
  if(digitalRead(buttons[2]) == HIGH && digitalRead(buttons[3]) == HIGH) {
    setAll(LOW);
    Serial.printf("All On\n");
    webSocket.broadcastTXT(formatStatus().c_str());
    delay(DELAY);
  }

  for(int i=0;i<TOTAL;i++) {
    if(digitalRead(buttons[i]) == HIGH) {
      digitalWrite(leds[i], !states[i]);
      Serial.printf("LED %i is %i\n", i+1, states[i]);
      webSocket.broadcastTXT(formatStatus().c_str());
      delay(DELAY);
    }
  }

}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
    case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      webSocket.sendTXT(num, formatStatus().c_str());
    }
    break;
    case WStype_TEXT: {
      Serial.printf("[%u] get Text: %s\n", num, payload);
      StaticJsonBuffer<512> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      String type = root["type"];
      if(type == "TOGGLE") {
        int pin = root["pin"];
        Serial.printf("Pin is %i",pin);
        int i = pin - 1;
        digitalWrite(leds[i], !digitalRead(leds[i]));
        webSocket.broadcastTXT(formatStatus().c_str());
        break;
      }
    }
    case WStype_BIN:
    Serial.printf("[%u] get binary length: %u\n", num, length);
    hexdump(payload, length);
    break;
  }
}
/*
* Custom Functions
*/
void setAll(int value) {
  for(int i=0;i<TOTAL;i++) {
    digitalWrite(leds[i], value);
  }
}
void updateStates() {
  for(int i=0;i<TOTAL;i++) {
    states[i] = digitalRead(leds[i]);
  }
}
String formatStatus() {
  updateStates();
  // {"type":"STATUS","data":"{"1":"1","2":"1","3":"1","4":"1"}"}
  String data = "{\"type\":\"STATUS\", \"data\":{";

  data += "\"1\":\"";
  data += !states[0];
  data += "\",";

  data += "\"2\":\"";
  data += !states[1];
  data += "\",";

  data += "\"3\":\"";
  data += !states[2];
  data += "\",";

  data += "\"4\":\"";
  data += !states[3];
  data += "\"";

  data += "}}";
  return data;
}

