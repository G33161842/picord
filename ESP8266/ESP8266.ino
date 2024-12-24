#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN D2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid[] = {"applepie", "CHANG"};
const char* password[] = {"000010000", "063316756"};
const int wifiNetworks = 2;

const char* serverUrl = "https://3.1.140.221:10001/upload.php";

void setup() {
  Serial.begin(115200);

   Wire.begin(D3, D4);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  
  bool connected = false;
  
  for (int i = 0; i < wifiNetworks; i++) {
    Serial.print("try to connect wifi：");
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
      Serial.print("connect ");
      Serial.println(ssid[i]);
      lcd.setCursor(0, 1);
      lcd.print("WiFi: ");
      lcd.print(ssid[i]);
      connected = true;
      break;  // 成功連接後退出循環
    } else {
      Serial.println("");
      Serial.println("connect fail,try next");
    }
  }
  
  if (!connected) {
    Serial.println("can't connect any wifi！");
    lcd.setCursor(0, 1);
    lcd.print("WiFi failed!");
    return;
  }

  dht.begin();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, serverUrl);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temp) || isnan(humidity)) {
      Serial.println("cant read DHT data");
      return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print("C");
    
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");

    String postData = "temperature=" + String(temp) + "&humidity=" + String(humidity);
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.println("send DHT data wrong");
    }

    http.end();
  }
  
  delay(10000);  // 每10秒發送一次數據
}