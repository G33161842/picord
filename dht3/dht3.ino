#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

const int DHTPIN = D7;
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid[] = {"daniu0807", "picord"};
const char* password[] = {"33161842", "000010000"};
const int wifiNetworks = 2;
const char* server = "http://redweb.magicboy.xyz";  // PHP 伺服器 IP

void setup() {
  Serial.begin(115200);
  dht.begin();

  bool connected = false;

  for (int i = 0; i < wifiNetworks; i++) {
    Serial.print("try to connect wifi: ");
    Serial.println(ssid[i]);
    WiFi.begin(ssid[i], password[i]);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 10) {
      delay(1000);
      Serial.print(".");
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid[i]);
      connected = true;
      break;  // 成功連接後退出循環
    } else {
      Serial.println("");
      Serial.println("Connect fail, try next");
      delay(1000);
    }
  }

  if (!connected) {
    Serial.println("Can't connect any WiFi!");
    return;
  }
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  HTTPClient http;
  WiFiClient client;
  String url = "http://redweb.magicboy.xyz/dht_update.php?id=3&temperature=" + String(temperature) + "&humidity=" + String(humidity);

  http.begin(client, url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("HTTP GET code: %d\n", httpCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.println("HTTP request failed!");
  }
  http.end();


  delay(60000);  // 每分鐘傳送一次數據
}
