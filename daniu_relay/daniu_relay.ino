#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED 顯示設定
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// 创建 OLED 对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT 感測器設定
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi 設定
const char* ssid[] = {"daniu0807", "kevin55688", "picord"};
const char* password[] = {"33161842", "picord55688", "000010000"};
const int wifiNetworks = 3;

// 控制變數
int fan1 = 0;
int autoMode = 0;
const float thresholdTemphigh = 28.0;
const float thresholdTemplow = 23.0;
float cachedTemp = 0.0;
float cachedHumi = 0.0;
unsigned long lastRead = 0;

#define RELAY_PIN D6  // 與 DHT 共用 D5，改接 D6 (GPIO12)

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
        return;
      } else {
        Serial.println("\n連接失敗，嘗試下一個 WiFi...");
      }
    }
    Serial.println("無法連接到任何 WiFi，5 秒後重新嘗試...");
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  connectWiFi();

  // 初始化 OLED 屏幕
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // 停止程序
  }
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to Wi-Fi...");
  display.display();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Wi-Fi Connected!");
  display.print("IP Address: ");
  display.println(WiFi.localIP());  // 显示 IP 地址
  display.display();
  
  // Web 控制
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"temp\":" + String(cachedTemp, 1) + ",\"humi\":" + String(cachedHumi, 1) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>即時溫濕度</title><style>";
    html += "body { font-family: Arial; text-align: center; padding: 20px; background-color: #f5f5f5; margin: 0; }";
    html += ".button { display: block; width: 90%; max-width: 350px; padding: 15px; margin: 10px auto; font-size: 20px; border-radius: 10px; }";
    html += ".button1 { background-color: #DCDCDC; } .button1:hover { background-color: #A9A9A9; }";
    html += ".button2 { background-color: #87CEEB; } .button2:hover { background-color: #4682B4; }";
    html += ".button4 { background-color: #F4A261; } .button4:hover { background-color: #D97F37; }";
    html += ".button5 { background-color: #E1D700; } .button5:hover { background-color: #C8B900; }";
    html += "</style></head><body>";
    html += "<h1>即時溫濕度</h1><div id='data'>載入中...</div>";
    html += "<h1>風扇控制</h1>";
    html += "<button class='button button1' onclick=\"sendRequest('on')\">開啟風扇</button>";
    html += "<button class='button button2' onclick=\"sendRequest('off')\">關閉風扇</button>";
    html += "<button class='button button4' onclick=\"sendRequest('onauto')\">自動模式開啟</button>";
    html += "<button class='button button5' onclick=\"sendRequest('offauto')\">自動模式關閉</button>";
    html += "<script>";
    html += "function sendRequest(url) { var xhr = new XMLHttpRequest(); xhr.open('GET', url, true); xhr.send(); }";
    html += "function updateData() { fetch('/data').then(r => r.json()).then(data => {";
    html += "document.getElementById('data').innerHTML = '目前溫度：' + data.temp.toFixed(1) + '°C<br>' + '目前濕度：' + data.humi.toFixed(1) + '%';";
    html += "}).catch(error => { document.getElementById('data').innerHTML = '資料讀取失敗'; console.error(error); }); }";
    html += "setInterval(updateData, 10000); updateData();";
    html += "</script></body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    fan1 = 1; autoMode = 0;
    digitalWrite(RELAY_PIN, HIGH);
    request->send(200, "text/plain", "風扇開啟");
    Serial.println("風扇開啟");
  });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    fan1 = 0; autoMode = 0;
    digitalWrite(RELAY_PIN, LOW);
    request->send(200, "text/plain", "風扇關閉");
    Serial.println("風扇關閉");
  });
  server.on("/onauto", HTTP_GET, [](AsyncWebServerRequest *request){
    autoMode = 1;
    request->send(200, "text/plain", "風扇自動模式開啟");
    Serial.println("風扇自動模式開啟");
  });
  server.on("/offauto", HTTP_GET, [](AsyncWebServerRequest *request){
    autoMode = 0;
    request->send(200, "text/plain", "風扇自動模式關閉");
    Serial.println("風扇自動模式關閉");
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

  if (autoMode == 1) {
    float temperature = dht.readTemperature();
    if (!isnan(temperature)) {
      if (temperature >= thresholdTemphigh && fan1 == 0) {
        fan1 = 1;
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("自動模式：溫度過高，風扇開啟");
      } else if (temperature < thresholdTemplow && fan1 == 1) {
        fan1 = 0;
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("自動模式：溫度過低，風扇關閉");
      }
    }
  }

  unsigned long now = millis();
  if (now - lastRead > 10000) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      cachedTemp = t;
      cachedHumi = h;
      Serial.printf("更新溫濕度: %.1f°C, %.1f%%\n", cachedTemp, cachedHumi);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("IP: ");
      display.println(WiFi.localIP());
      display.print("Temp: ");
      display.print(cachedTemp, 1);
      display.println(" C");
      display.print("Humi: ");
      display.print(cachedHumi, 1);
      display.println(" %");
      display.display();
    } else {
      Serial.println("讀取失敗");
    }
    lastRead = now;
  }
}
