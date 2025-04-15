#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTPIN D2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid[] = {"daniu0807", "CHANG"};
const char* password[] = {"33161842", "063316756"};
const int wifiNetworks = 2;

const char* serverUrl = "https://3.1.140.221:10011/upload.php";
const int irPin1 = D5;
const int irPin2 = D6;
const int irPin3 = D7;
IRsend irsend1(irPin1);
IRsend irsend2(irPin2);
IRsend irsend3(irPin3);

int fan1 = 0;
int fan2 = 0;
int fan3 = 0;
int autoMode = 0;
const float thresholdTemphigh = 28.0;
const float thresholdTemplow = 25.0;

AsyncWebServer server(80);

void connectToWiFi() {
  bool connected = false;
  for (int i = 0; i < wifiNetworks; i++) {
    Serial.print("Trying to connect to Wi-Fi: ");
    Serial.println(ssid[i]);

    WiFi.begin(ssid[i], password[i]);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 10) {
      delay(1000);
      Serial.print(".");
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to Wi-Fi: ");
      Serial.println(ssid[i]);
      connected = true;
      break;
    } else {
      Serial.println("\nConnection failed, trying next...");
    }
  }

  if (!connected) {
    Serial.println("Unable to connect to any Wi-Fi network!");
    while (1);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  irsend1.begin();
  irsend2.begin();
  irsend3.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to Wi-Fi...");
  display.display();

  connectToWiFi();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Wi-Fi Connected!");
  display.print("IP Address: ");
  display.println(WiFi.localIP());
  display.display();

  // Set up web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Fan Control</title>";
    html += "<style>body { font-family: Arial; text-align: center; padding: 20px; background: #f5f5f5; }";
    html += ".button { display: block; width: 80%; max-width: 300px; margin: 10px auto; padding: 10px; background: #28a745; color: white; font-size: 20px; border: none; border-radius: 5px; }";
    html += ".button:hover { background: #218838; }</style></head><body>";
    html += "<h1>Fan Control</h1>";
    html += "<button class='button' onclick=\"sendRequest('/on1')\">Fan 1 ON</button>";
    html += "<button class='button' onclick=\"sendRequest('/off1')\">Fan 1 OFF</button>";
    html += "<button class='button' onclick=\"sendRequest('/on2')\">Fan 2 ON</button>";
    html += "<button class='button' onclick=\"sendRequest('/off2')\">Fan 2 OFF</button>";
    html += "<button class='button' onclick=\"sendRequest('/on3')\">Fan 3 ON</button>";
    html += "<button class='button' onclick=\"sendRequest('/off3')\">Fan 3 OFF</button>";
    html += "<button class='button' onclick=\"sendRequest('/auto')\">Enable Auto Mode</button>";
    html += "<button class='button' onclick=\"sendRequest('/manual')\">Switch to Manual Mode</button>";
    html += "<script>function sendRequest(url) { fetch(url).then(resp => resp.text()).then(txt => alert(txt)); }</script></body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/on1", HTTP_GET, [](AsyncWebServerRequest *request) {
    irsend1.sendSymphony(0xD82, 12);
    fan1 = 1;
    request->send(200, "text/plain", "Fan 1 is ON");
  });
  server.on("/off1", HTTP_GET, [](AsyncWebServerRequest *request) {
    irsend1.sendSymphony(0xD81, 12);
    fan1 = 0;
    request->send(200, "text/plain", "Fan 1 is OFF");
  });
  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request) {
    autoMode = 1;
    request->send(200, "text/plain", "Auto mode enabled");
  });
  server.on("/manual", HTTP_GET, [](AsyncWebServerRequest *request) {
    autoMode = 0;
    request->send(200, "text/plain", "Manual mode enabled");
  });
  

  server.begin();
}

void loop() {
  if (autoMode == 1) {
    float temperature = dht.readTemperature();
    if (!isnan(temperature)) {
      if (temperature >= thresholdTemphigh && fan1 == 0) {
        irsend1.sendSymphony(0xD82, 12);
        fan1 = 1;
      } else if (temperature < thresholdTemplow && fan1 == 1) {
        irsend1.sendSymphony(0xD81, 12);
        fan1 = 0;
      }
    }
  }
}
