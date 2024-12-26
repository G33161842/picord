#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RECEIVER_PIN  D5  // 红外接收器的连接引脚
#define SCREEN_WIDTH 128  // OLED 屏幕宽度
#define SCREEN_HEIGHT 64  // OLED 屏幕高度
#define OLED_RESET    -1  // OLED 重置引脚，通常设置为 -1 不使用

IRrecv irrecv(RECEIVER_PIN);   // 创建红外接收器实例
decode_results results;        // 存储解码后的红外信号

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // 创建 OLED 屏幕实例

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();  // 初始化红外接收器
  
  // 初始化 OLED 屏幕
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 OLED 屏幕初始化失败"));
    while (true);
  }
  display.display();   // 显示初始化的屏幕
  delay(2000);         // 延时 2 秒
}

void loop() {
  if (irrecv.decode(&results)) {   // 如果接收到红外信号
    Serial.println(resultToHumanReadableBasic(&results));  // 打印解码结果
    display.clearDisplay();        // 清除屏幕内容
    display.setTextSize(1);        // 设置字体大小
    display.setTextColor(SSD1306_WHITE);  // 设置字体颜色为白色
    display.setCursor(0, 0);       // 设置文本起始位置
    display.print(F("Received IR Signal:"));
    display.setCursor(0, 10);
    display.print(resultToHumanReadableBasic(&results));  // 显示信号内容
    display.display();             // 更新显示屏内容
    irrecv.resume();  // 准备接收下一个信号
  }
}
