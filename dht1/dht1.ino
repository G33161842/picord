#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "picord";
const char* password = "000010000";
const char* server = "http://redweb.magicboy.xyz";  // PHP 伺服器 IP

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  WiFiClient client;
  String url = "/dht_update.php?id=1&temperature=" + String(temperature) + "&humidity=" + String(humidity);

  Serial.print("Requesting URL: ");
  Serial.println(url);

  if (client.connect(server, 80)) {
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(500);

    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
  } else {
    Serial.println("Connection failed!");
  }

  delay(60000);  // 每分鐘傳送一次數據
}
