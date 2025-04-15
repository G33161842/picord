#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 定义 OLED 屏幕的尺寸
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 定义 I2C 引脚
#define OLED_RESET -1  // 可选择使用 -1 连接至 RST
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 設置您的手機 Wi-Fi 網路名稱和密碼
const char* ssid[] = {"daniu0807", "picord"};
const char* password[] = {"33161842", "000010000"};
const int wifiNetworks = 2;

// 初始化紅外發射器
IRsend irsend(D7);  // 將 D7 連接到紅外發射器
#define RELAY_PIN D5

AsyncWebServer server(80);

void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < wifiNetworks; i++) {
      Serial.print("嘗試連接 WiFi: ");
      Serial.println(ssid[i]);
      // 更新 OLED 显示内容
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Connecting to Wi-Fi..");
      //display.println(ssid[i]); // 显示当前尝试的 SSID
      display.display();

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

        // 更新 OLED 屏幕显示
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("WiFi Connected!");
        display.println();
        display.print("SSID:");
        display.println(ssid[i]);
        display.println();
        display.print("IP:");
        display.println(WiFi.localIP());
        display.display();
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
  // 初始化 OLED 显示屏
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 使用 I2C 地址 0x3C
      Serial.println("OLED 显示屏初始化失败!");
      for (;;); // 如果初始化失败，停止运行
  }
  irsend.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 繼電器初始為關閉
  connectWiFi(); // 呼叫連接 WiFi 函數

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>暖氣與風扇控制</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; background-color: #f5f5f5; margin: 0; }";
    html += "h1 { color: #333; font-size: 28px; margin-bottom: 20px; }";
    html += ".button { display: block; width: 90%; max-width: 350px; padding: 15px; margin: 10px auto; font-size: 20px; color: black; border: none; border-radius: 10px; cursor: pointer; text-align: center; transition: background-color 0.3s ease; }";
    html += ".button1 { background-color: #DCDCDC; } .button1:hover { background-color: #A9A9A9; }";
    html += ".button2 { background-color: #87CEEB; } .button2:hover { background-color: #4682B4; }";
    html += ".button3 { background-color: #FF7F7F; } .button3:hover { background-color: #FF0000; }";
    html += ".button4 { background-color: #F4A261; } .button4:hover { background-color: #D97F37; }";
    html += ".button5 { background-color: #E1D700; } .button5:hover { background-color: #C8B900; }";
    html += ".button6 { background-color: #90EE90; } .button6:hover { background-color: #00FF00; }";
    html += ".button7 { background-color: #FF7F7F; } .button7:hover { background-color: #FF0000; }";
    html += ".button8 { background-color: #90EE90; } .button6:hover { background-color: #00FF00; }";
    html += ".button9 { background-color: #FF7F7F; } .button7:hover { background-color: #FF0000; }";
    html += "</style></head><body>";

    html += "<h1>暖氣控制</h1>";
    html += "<button class='button button1' onclick=\"sendRequest('on')\">開關暖氣</button>";
    html += "<button class='button button2' onclick=\"sendRequest('down')\">降低溫度</button>";
    html += "<button class='button button3' onclick=\"sendRequest('up')\">提升溫度</button>";
    html += "<button class='button button4' onclick=\"sendRequest('speed')\">改變風速：LL低HH高</button>";
    html += "<button class='button button5' onclick=\"sendRequest('time')\">定時功能</button>";

    html += "<h1>風扇控制</h1>";
    html += "<button class='button button6' onclick=\"sendRequest('relayon')\">開啟風扇</button>";
    html += "<button class='button button7' onclick=\"sendRequest('relayoff')\">關閉風扇</button>";

    html += "<script>";
    html += "function controlFan(action) {";
    html += "  var xhr = new XMLHttpRequest();";
    html += "  xhr.open('GET', 'http://192.168.55.189/' + action, true);"; // 發送請求至指定路徑
    html += "  xhr.send();";
    html += "}";

    html += "function sendRequest(url) {";
    html += "  var xhr = new XMLHttpRequest();";
    html += "  xhr.open('GET', url, true);";
    html += "  xhr.send();";
    html += "}";
    html += "</script></body></html>";

    request->send(200, "text/html", html);
  });

  // 處理按鈕點擊事件
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    irsend.sendNEC(0xFF02FD, 32);  // 發送紅外信號開啟或關閉暖氣
    request->send(200, "text/plain", "暖氣已開啟或關閉");
    Serial.println("暖氣已開啟或關閉");
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
    request->send(200, "text/plain", "風速已改變");
    Serial.println("風速已改變");
  });
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    irsend.sendNEC(0xFF7A85, 32);  // 發送紅外信號設置時間
    request->send(200, "text/plain", "時間已設置");
    Serial.println("時間已設置");
  });
  server.on("/relayon", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, HIGH); // 繼電器開啟
    request->send(200, "text/plain", "風扇已開啟");
    Serial.println("風扇已開啟");
  });
  server.on("/relayoff", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, LOW); // 繼電器關閉
    request->send(200, "text/plain", "風扇已關閉");
    Serial.println("風扇已關閉");
  });
  // 啟動 Web 伺服器
  server.begin();
}

void loop() {
  static unsigned long lastReconnectAttempt = 0;
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 10000) { // 每 10 秒尝试重连
      lastReconnectAttempt = now;
      Serial.println("WiFi 斷開，嘗試重新連接...");
      connectWiFi(); // 再次尝试连接 Wi-Fi
    }
  }
}