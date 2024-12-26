#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// 設置您的手機 Wi-Fi 網路名稱和密碼
const char* ssid[] = {"daniu0807", "picord"};
const char* password[] = {"33161842", "000010000"};
const int wifiNetworks = 2;

// 初始化紅外發射器
IRsend irsend(D5);  // 將 D5 連接到紅外發射器

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
  connectWiFi(); // 呼叫連接 WiFi 函數

  // 創建 Web 頁面
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>暖氣控制</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; background-color: #f5f5f5; margin: 0; }";
    html += "h1 { color: #333; font-size: 28px; margin-bottom: 20px; }";

    // 按鈕的基本樣式
    html += ".button {";
    html += "  display: block; width: 100%; max-width: 350px; padding: 20px; margin: 15px auto;";
    html += "  font-size: 26px; color: black; border: none; border-radius: 15px;";
    html += "  cursor: pointer; text-align: center; transition: background-color 0.3s ease; }";

    // 按鈕顏色設置
    html += ".button1 { background-color: #DCDCDC; }";  // 白色 -> 淺灰色（更深）
    html += ".button2 { background-color: #87CEEB; }";  // 淡藍色 -> 藍色（加深）
    html += ".button3 { background-color: #FF69B4; }";  // 淡紅色 -> 熱情紅（加深）
    html += ".button4 { background-color: #F4A261; }";  // 皮膚色 -> 橙色（加深）
    html += ".button5 { background-color: #E1D700; }";  // 淡黃色 -> 黃色（加深）

    // 按鈕懸停顏色
    html += ".button1:hover { background-color: #A9A9A9; }";  // 白色 -> 深灰色
    html += ".button2:hover { background-color: #4682B4; }";  // 淡藍色 -> 鋼藍色
    html += ".button3:hover { background-color: #FF1493; }";  // 淡紅色 -> 深粉紅
    html += ".button4:hover { background-color: #D97F37; }";  // 皮膚色 -> 深橙色
    html += ".button5:hover { background-color: #C8B900; }";  // 淡黃色 -> 深黃色

    // 頁面自適應
    html += "@media screen and (max-width: 600px) {";
    html += "  h1 { font-size: 24px; }";  // 標題字體大小調整
    html += "  .button { font-size: 22px; padding: 25px; }";  // 按鈕字體和間距調整
    html += "}";

    html += "#status { font-size: 20px; margin-top: 20px; color: #333; }";
    html += "</style></head><body>";

    html += "<h1>暖氣控制</h1>";
    html += "<button class='button button1' onclick=\"sendRequest('on')\">開啟暖氣</button><br>";
    html += "<button class='button button2' onclick=\"sendRequest('down')\">降低溫度</button><br>";
    html += "<button class='button button3' onclick=\"sendRequest('up')\">提升溫度</button><br>";
    html += "<button class='button button4' onclick=\"sendRequest('speed')\">改變速度</button><br>";
    html += "<button class='button button5' onclick=\"sendRequest('time')\">設定時間</button><br>";
    //html += "<div id='status'></div>"; // 顯示狀態

    html += "<script>";
    html += "function sendRequest(url) {";
    html += "  var xhr = new XMLHttpRequest();";
    html += "  xhr.open('GET', url, true);";
    html += "  xhr.onload = function() {";
    html += "    if (xhr.status === 200) {";
    html += "      document.getElementById('status').innerHTML = xhr.responseText;";
    html += "    }";
    html += "  };";
    html += "  xhr.send();";
    html += "}";
    html += "</script></body></html>";

    request->send(200, "text/html", html);
  });

  // 處理按鈕點擊事件
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    irsend.sendNEC(0xFF02FD, 32);  // 發送紅外信號開啟或關閉暖氣
    request->send(200, "text/plain", "暖氣已開啟");
    Serial.println("暖氣已開啟");
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    irsend.sendNEC(0xFFE01F, 32);  // 發送紅外信號降低溫度
    request->send(200, "text/plain", "溫度已降低");
    Serial.println("溫度已降低");
  });

  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request){
    irsend.sendNEC(0xFF906F, 32);  // 發送紅外信號提升溫度
    request->send(200, "text/plain", "溫度已提升");
    Serial.println("溫度已提升");
  });

  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest *request){
    irsend.sendNEC(0xFF30CF, 32);  // 發送紅外信號改變速度
    request->send(200, "text/plain", "速度已改變");
    Serial.println("速度已改變");
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    irsend.sendNEC(0xFF7A85, 32);  // 發送紅外信號設置時間
    request->send(200, "text/plain", "時間已設置");
    Serial.println("時間已設置");
  });

  // 啟動 Web 伺服器
  server.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi 斷開，嘗試重新連接...");
    connectWiFi(); // 呼叫重新連接函數
  }
}