#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>


const uint16_t recv_pin = D2;  // ESP8266 D2 腳連接 IR 接收器

IRrecv irrecv(recv_pin);  // 設置接收引腳
decode_results results;  // 儲存解碼後的 IR 信號

void setup() {
  Serial.begin(115200);  // 初始化串口監視器
  irrecv.enableIRIn();  // 啟動 IR 接收功能
  Serial.println("红外接收器已启用");
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(resultToHumanReadableBasic(&results)); // 打印红外信号
    irrecv.resume(); // 准备接收下一个信号
  }
}
