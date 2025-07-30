#include <MechaQMC5883.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>

MechaQMC5883 qmc;
RTC_DS3231 rtc;

#define XBee Serial1

int x, y, z;
int num = 0;
int azimuth;
bool isRunning = false; // 控制是否開始傳輸資料

time_t getTimeFromRTC() {
  DateTime now = rtc.now();
  return now.unixtime();
}

void setup() {
  Serial.begin(9600);
  XBee.begin(9600);
  Wire.begin();
  qmc.init();

  setSyncProvider(getTimeFromRTC);

  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  // 顯示說明訊息
  Serial.println("Send 'y' to start, 'n' to stop.");
  XBee.println("Send 'y' to start, 'n' to stop.");

  // 印出 CSV 標頭列
  Serial.println("Time,Num,X,Y,Z,Magnitude,heading");
  XBee.println("Time,Num,X,Y,Z,Magnitude,heading");
}

void loop() {
  // 檢查是否收到 XBee 指令
  if (XBee.available()) {
    char cmd = XBee.read();
    if (cmd == 'y') {
      isRunning = true;
      Serial.println("Start reading...");
      XBee.println("Start reading...");
    } else if (cmd == 'n') {
      isRunning = false;
      Serial.println("Stop reading...");
      XBee.println("Stop reading...");
      while (!XBee.available()) {
        // 完全暫停，直到接收到新指令（例如 'y'）
        delay(100);
      }
      return; // 跳過本次 loop 迴圈，避免誤執行後續感測器動作
    }
  }

  if (!isRunning) return;  // 若未啟動，直接跳過以下區塊

  // 資料讀取與傳送
  qmc.read(&x, &y, &z, &azimuth);

  float fx = x;
  float fy = y;
  float fz = z;
  float magnitude = sqrt(fx * fx + fy * fy + fz * fz);
  float heading = azimuth;
  int mag5 = find_nearest_multiple_of_5(magnitude);

  char timeBuffer[10];
  sprintf(timeBuffer, "%02d:%02d:%02d", hour(), minute(), second());

  char xBuf[6], yBuf[6], zBuf[6];
  formatFourDigit(x, xBuf);
  formatFourDigit(y, yBuf);
  formatFourDigit(z, zBuf);

  String data = String(timeBuffer) + "," + num + "," +
                String(xBuf) + "," + String(yBuf) + "," + String(zBuf) + "," + magnitude + "," + heading;

  Serial.println(data);
  XBee.println(data);

  num++;
  delay(500);
}

// 將 int 格式化為四位數（含正負號），存入 char[]
void formatFourDigit(int val, char* buffer) {
  char sign = (val >= 0) ? '+' : '-';
  val = abs(val);
  sprintf(buffer, "%c%04d", sign, val);  // 例如 +0123, -0456
}

// 找出最接近的 5 的倍數
int find_nearest_multiple_of_5(float number) {
  float remainder = fmod(number, 5.0);
  if (remainder == 0.0)
    return (int)number;
  else if (remainder < 2.5)
    return (int)(number - remainder);
  else
    return (int)(number + (5.0 - remainder));
}
