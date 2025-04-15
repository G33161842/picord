#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "picord";
const char* password = "000010000";

//const char* ssid = "daniu0807";
//const char* password = "33161842";

// 繼電器接的GPIO
#define RELAY_PIN D5

ESP8266WebServer server(80);

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
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 繼電器初始為關閉

  // 連接Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("連接到Wi-Fi中");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("連接成功");
  Serial.print("IP地址: ");
  Serial.println(WiFi.localIP());

  // 設置HTTP服務
  server.on("/", []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on("/on", []() {
    digitalWrite(RELAY_PIN, HIGH); // 繼電器開啟
    server.send(200, "text/plain", "繼電器已開啟");
  });

  server.on("/off", []() {
    digitalWrite(RELAY_PIN, LOW); // 繼電器關閉
    server.send(200, "text/plain", "繼電器已關閉");
  });

  server.begin();
  Serial.println("HTTP 服務啟動");
}

// 主循環
void loop() {
  server.handleClient();
}