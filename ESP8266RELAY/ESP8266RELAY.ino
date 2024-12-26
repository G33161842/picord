#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Wi-Fi 網路清單
const char* ssid[] = {"daniu0807", "picord"};
const char* password[] = {"33161842", "000010000"};
const int wifiNetworks = 2;

// 繼電器接的 GPIO
#define RELAY_PIN D5

// Web 伺服器
ESP8266WebServer server(80);

// 嘗試連接 Wi-Fi 的函數
void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < wifiNetworks; i++) {
      Serial.print("嘗試連接 Wi-Fi: ");
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
        Serial.print("已連接到 Wi-Fi: ");
        Serial.println(ssid[i]);
        Serial.print("IP 地址: ");
        Serial.println(WiFi.localIP());
        return; // 成功連接後退出函數
      } else {
        Serial.println("\n連接失敗，嘗試下一個 Wi-Fi...");
      }
    }
    Serial.println("無法連接到任何 Wi-Fi，5 秒後重新嘗試...");
    delay(5000); // 等待後重試
  }
}

// 優化為手機友好的中文控制頁面HTML
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>繼電器控制</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; }
    h1 { background-color: #4CAF50; color: white; padding: 10px; margin: 0; }
    .button {
      display: inline-block;
      background-color: #008CBA;
      color: white;
      padding: 15px 30px;
      font-size: 20px;
      margin: 20px 10px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      box-shadow: 2px 2px 5px rgba(0,0,0,0.3);
    }
    .button:hover { background-color: #005f6b; }
  </style>
</head>
<body>
  <h1>ESP8266 繼電器控制</h1>
  <button class="button" onclick="fetch('on')">開啟繼電器</button>
  <button class="button" onclick="fetch('off')">關閉繼電器</button>
</body>
</html>
)rawliteral";

// 初始化Wi-Fi與繼電器
void setup() {
  Serial.begin(115200);
  connectWiFi(); // 呼叫連接 Wi-Fi 函數
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 繼電器初始為關閉

  // 設置HTTP服務
  server.on("/", []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on("/on", []() {
    digitalWrite(RELAY_PIN, HIGH); // 繼電器開啟
    server.send(200, "text/plain", "繼電器已開啟");
    Serial.println("繼電器已開啟");
  });

  server.on("/off", []() {
    digitalWrite(RELAY_PIN, LOW); // 繼電器關閉
    server.send(200, "text/plain", "繼電器已關閉");
    Serial.println("繼電器已關閉");
  });

  server.begin();
  Serial.println("HTTP 服務啟動");
}

// 主循環
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi 斷開，嘗試重新連接...");
    connectWiFi(); // 呼叫重新連接函數
  }
  server.handleClient();
}
