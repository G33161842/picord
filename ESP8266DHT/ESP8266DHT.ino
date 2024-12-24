#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define DHTPIN 6          // DHT11 数据引脚
#define DHTTYPE DHT11     // 使用 DHT11 温湿度传感器

const char* ssid = "picord";          // WiFi 名稱
const char* password = "000010000";  // WiFi 密碼

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

bool autoControl = false;  // 自動控制開關
bool lastFanState = false; // 上次風扇狀態，用於避免重複發送 API 請求
unsigned long lastSensorRead = 0;
unsigned long sensorReadInterval = 5000; // 傳感器讀取間隔（毫秒）

void setup() {
  Serial.begin(115200);
  connectWiFi();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi, restarting...");
    ESP.restart();
  }

  Serial.println("Connected to WiFi");
  dht.begin();

  server.on("/auto", HTTP_GET, []() {
    autoControl = true;
    server.send(200, "text/plain", "Auto mode activated");
  });

  server.on("/disauto", HTTP_GET, []() {
    autoControl = false;
    server.send(200, "text/plain", "Auto mode deactivated");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, attempting to reconnect...");
    connectWiFi();
  }

  if (autoControl && millis() - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = millis();
    handleTemperatureControl();
  }

  delay(1); // 喂看門狗
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500); // 每次等待 500ms
    Serial.print(".");
    yield(); // 喂看門狗，避免觸發 WDT Reset
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
  } else {
    Serial.println("Failed to connect to WiFi");
  }
}

void handleTemperatureControl() {
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed to read temperature!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.println(temperature);

  if (temperature > 28 && !lastFanState) {
    sendApiRequest("https://redweb.magicboy.xyz/api/red2_on");
    lastFanState = true;
  } else if (temperature < 25 && lastFanState) {
    sendApiRequest("https://redweb.magicboy.xyz/api/red2_off");
    lastFanState = false;
  }
}

void sendApiRequest(const char* url) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.print("API request successful: ");
    Serial.println(url);
  } else {
    Serial.print("Failed to send API request: ");
    Serial.println(url);
  }
  
  http.end();
}
