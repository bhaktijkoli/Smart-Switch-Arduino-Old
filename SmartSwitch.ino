#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81);

int button1 = D5;
int button2 = D6;
int button3 = D7;
int button4 = D8;

int led1 = D1;
int led2 = D2;
int led3 = D3;
int led4 = D4;

int state1 = LOW;
int state2 = LOW;
int state3 = LOW;
int state4 = LOW;

int delaySecs = 2000;

void setup() {
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  Serial.println("Started");
  WiFiMulti.addAP("NarutoOT", "asdfghjkl");
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
  
  state1 = digitalRead(led1);
  state2 = digitalRead(led2);
  state3 = digitalRead(led3);
  state4 = digitalRead(led4);
  return;

  if(digitalRead(button1) == HIGH && digitalRead(button2) == HIGH) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
    Serial.printf("All Off\n");
    delay(delaySecs);
  }
  if(digitalRead(button3) == HIGH && digitalRead(button4) == HIGH) {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    Serial.printf("All On\n");
    delay(delaySecs);
  }
  

  if(digitalRead(button1) == HIGH) {
    digitalWrite(led1, !state1);
    Serial.printf("LED 1 is %i\n", state1);
    webSocket.broadcastTXT(formatStatus().c_str());
    delay(delaySecs);
  }
  if(digitalRead(button2) == HIGH) {
    digitalWrite(led2, !state2);
    Serial.printf("LED 2 is %i\n", state2);
    webSocket.broadcastTXT(formatStatus().c_str());
    delay(delaySecs);
  }
  if(digitalRead(button3) == HIGH) {
    digitalWrite(led3, !state3);
    Serial.printf("LED 3 is %i\n", state3);
    webSocket.broadcastTXT(formatStatus().c_str());
    delay(delaySecs);
  }
  if(digitalRead(button4) == HIGH) {
    digitalWrite(led4, !state4);
    Serial.printf("LED 4 is %i\n", state4);
    webSocket.broadcastTXT(formatStatus().c_str());
    delay(delaySecs);
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
              switch (pin) {
              case 1: digitalWrite(led1, !digitalRead(led1));
              break;
              case 2: digitalWrite(led2, !digitalRead(led2));
              break;
              case 3: digitalWrite(led3, !digitalRead(led3));
              break;
              case 4: digitalWrite(led4, !digitalRead(led4));
              break;
              }
            }
            webSocket.broadcastTXT(formatStatus().c_str());
            break;
        }
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);
            break;
    }
}

String formatStatus() {
  state1 = digitalRead(led1);
  state2 = digitalRead(led2);
  state3 = digitalRead(led3);
  state4 = digitalRead(led4);
  // {"type":"STATUS","data":"{"1":"1","2":"1","3":"1","4":"1"}"}
  String data = "{\"type\":\"STATUS\", \"data\":{";

  data += "\"1\":\"";
  data += !state1;
  data += "\",";

  data += "\"2\":\"";
  data += !state2;
  data += "\",";

  data += "\"3\":\"";
  data += !state3;
  data += "\",";

  data += "\"4\":\"";
  data += !state4;
  data += "\"";

  data += "}}";
  return data;
}
