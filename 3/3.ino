#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>
#include <Wire.h>

// 設置您的手機 Wi-Fi 網路名稱和密碼
const char* ssid[] = {"daniu0807","kevin55688","picord"};
const char* password[] = {"33161842","picord55688","000010000"};
const int wifiNetworks = 3;

#define RELAY_PIN D5

AsyncWebServer server(80);

void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < wifiNetworks; i++) {
      Serial.print("嘗試連接 WiFi: ");
      Serial.println(ssid[i]);
      WiFi.begin(ssid[i], password[i]);

      int retries = 0;
      while (WiFi.status() != WL_CONNECTED && retries < 10) {
        delay(1000);
        Serial.print(".");
        retries++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n連接成功!");
        Serial.print("已連接到 WiFi: ");
        Serial.println(ssid[i]);
        Serial.print("IP 地址: ");
        Serial.println(WiFi.localIP());
        return; // 成功連接後退出函數
      } else {
        Serial.println("\n連接失敗，嘗試下一個 WiFi...");
      }
    }
    Serial.println("無法連接到任何 WiFi，5 秒後重新嘗試...");
    delay(5000); // 等待後重試
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 初始關閉繼電器
  connectWiFi();
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, HIGH);
    request->send(200, "text/plain", "燈光已開啟");
    Serial.println("燈光已開啟");
  });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, LOW);
    request->send(200, "text/plain", "燈光已關閉");
    Serial.println("燈光已關閉");
  });
  server.begin();
}

void loop() {
  static unsigned long lastReconnectAttempt = 0;
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 10000) {
      lastReconnectAttempt = now;
      Serial.println("WiFi 斷開，嘗試重新連接...");
      connectWiFi();
    }
  }
}
