#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTTYPE DHT11
const int DHTPIN = D7;
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "picord";
const char* password = "000010000";
const char* server = "redweb.magicboy.xyz";

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
  String url = "/dht_update.php?action=update&id=3&temperature=" + String(temperature) + "&humidity=" + String(humidity);
  //String url = "/dht_update.php?id=3&temperature=" + String(temperature) + "&humidity=" + String(humidity);
  https://redweb.magicboy.xyz/dht_update.php?action=update&id=3&temperature=25&humidity=25

  Serial.print("Requesting URL: ");
  Serial.println(url);

  if (client.connect(server, 80)) {
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(500);

  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
    client.stop();
  } else {
    Serial.println("Connection failed!");
  }

  delay(60000);  // 每分鐘傳送一次數據
}
