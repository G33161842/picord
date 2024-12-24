#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h> // 引入 HTTPClient 庫

#define DHTPIN 7 // DHT11 資料接腳
#define DHTTYPE DHT11 // 使用 DHT11 溫濕度感測器

//const char* ssid = "picord";
//const char* password = "000010000";

const char* ssid = "picord";
const char* password = "000010000";

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

bool autoControl = false; // 自動控制開關

void setup() {
 Serial.begin(115200);

 // 連線到Wi-Fi
 WiFi.begin(ssid, password);
 Serial.print("連接到Wi-Fi中");
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("連線成功");
 Serial.print("IP位址: ");
 Serial.println(WiFi.localIP());

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

 // 只有在自動模式啟用時，才根據溫度控制 API
 if (autoControl) {
 float temperature = dht.readTemperature();
 if (isnan(temperature)) {
 Serial.println("讀取溫度失敗！");
 return;
 }

 Serial.print("溫度: ");
 Serial.println(temperature);

 if (temperature > 28) {
 sendApiRequest("https://redweb.magicboy.xyz/api/red3_on");
 } else if (temperature < 25) {
 sendApiRequest("https://redweb.magicboy.xyz/api/red3_off");
 }
 }
}

void sendApiRequest(const char* url) {
 WiFiClient client;
 HTTPClient http; // 建立 HTTPClient 對象
 http.begin(client, url);
 int httpCode = http.GET();

 if (httpCode > 0) {
 Serial.println("API 請求發送成功");
 } else {
 Serial.println("API 請求發送失敗");
 }

 http.end();
}