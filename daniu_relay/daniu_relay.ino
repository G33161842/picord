#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>
#include <Wire.h>

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 設置您的手機 Wi-Fi 網路名稱和密碼
const char* ssid[] = {"daniu0807","kevin55688","picord"};
const char* password[] = {"33161842","picord55688","000010000"};
const int wifiNetworks = 3;

int fan1 = 0;
int autoMode = 0;
const float thresholdTemphigh = 28.0;
const float thresholdTemplow = 23.0;
float cachedTemp = 0.0;
float cachedHumi = 0.0;
unsigned long lastRead = 0;

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
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 初始關閉繼電器
  connectWiFi();

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"temp\":" + String(cachedTemp, 1) + ",\"humi\":" + String(cachedHumi, 1) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>即時溫濕度</title><style>";
    html += "body { font-family: Arial; text-align: center; padding: 20px; background-color: #f5f5f5; margin: 0; }";
    html += "h1 { color: #333; font-size: 28px; margin-bottom: 20px; }";
    html += ".button { display: block; width: 90%; max-width: 350px; padding: 15px; margin: 10px auto; font-size: 20px; color: black; border: none; border-radius: 10px; cursor: pointer; text-align: center; transition: background-color 0.3s ease; }";
    html += ".button1 { background-color: #DCDCDC; } .button1:hover { background-color: #A9A9A9; }";
    html += ".button2 { background-color: #87CEEB; } .button2:hover { background-color: #4682B4; }";
    html += ".button3 { background-color: #FF7F7F; } .button3:hover { background-color: #FF0000; }";
    html += ".button4 { background-color: #F4A261; } .button4:hover { background-color: #D97F37; }";
    html += ".button5 { background-color: #E1D700; } .button5:hover { background-color: #C8B900; }";
    html += ".button6 { background-color: #90EE90; } .button6:hover { background-color: #00FF00; }";
    html += ".button7 { background-color: #FF7F7F; } .button7:hover { background-color: #FF0000; }";
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
    autoMode = 1; request->send(200, "text/plain", "風扇自動模式開啟");
    Serial.println("風扇自動模式開啟");
  });
  server.on("/offauto", HTTP_GET, [](AsyncWebServerRequest *request){
    autoMode = 0; request->send(200, "text/plain", "風扇自動模式關閉");
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
      if (temperature >= thresholdTemphigh && fan1 == 0) {//28
        fan1 = 1;
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("自動模式：溫度過高，風扇開啟");
      } else if (temperature < thresholdTemplow && fan1 == 1) {//23
        fan1 = 0;
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("自動模式：溫度過低，風扇關閉");
      }
    }
  }

  unsigned long now = millis();
  if (now - lastRead > 10000) {//每10秒更新溫溼度
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      cachedTemp = t;
      cachedHumi = h;
      Serial.printf("更新溫濕度: %.1f°C, %.1f%%\n", cachedTemp, cachedHumi);
    } else {
      Serial.println("讀取失敗");
    }
    lastRead = now;
  }
}
