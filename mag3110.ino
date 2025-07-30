#include <SparkFun_MAG3110.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>
#include <math.h>

MAG3110 mag;
RTC_DS3231 rtc;

#define XBee Serial1

int x, y, z;
int num = 0;
bool isRunning = false; // 控制是否開始傳輸資料

time_t getTimeFromRTC() {
  DateTime now = rtc.now();
  return now.unixtime();
}

void setup() {
  Serial.begin(9600);
  XBee.begin(9600);
  Wire.begin();

   mag.initialize();
  if (mag.isCalibrated()) {
    Serial.println("MAG3110 connected.");
  } else {
    Serial.println("MAG3110 not connected!");
  }

  mag.start();
  rtc.begin();
  setSyncProvider(getTimeFromRTC);

  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  Serial.println("Send 'y' to start, 'n' to stop.");
  XBee.println("Send 'y' to start, 'n' to stop.");

  Serial.println("Time,Num,X,Y,Z,Magnitude,heading");
  XBee.println("Time,Num,X,Y,Z,Magnitude,heading");
}

void loop() {
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
        delay(100);  // 等待下一個指令再恢復
      }
      return;
    }
  }

  if (!isRunning) return;
  mag.readMag(&x, &y, &z);

  float fx = x;
  float fy = y;
  float fz = z;
  float magnitude = sqrt(fx * fx + fy * fy + fz * fz);
  float heading = atan2(fy, fx) * 180.0 / PI;
  if (heading < 0) heading += 360;

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

// 將 int 格式化為四位數（含正負號）
void formatFourDigit(int val, char* buffer) {
  char sign = (val >= 0) ? '+' : '-';
  val = abs(val);
  sprintf(buffer, "%c%04d", sign, val);
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

