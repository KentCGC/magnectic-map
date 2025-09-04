#include <SparkFun_MAG3110.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>
#include <math.h>

MAG3110 mag;
RTC_DS3231 rtc;

#define XBee Serial1
long previousTime=0;
long interval= 300;
int x, y, z;
int num = 0;
bool isRunning = false;
int px = 0, py = 0, pz = 0;
float calz=0;
float calx=0;
float caly=0;
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
  Serial.println("Time,Num,X,Y,Z,Magnitude,Heading");
  XBee.println("Time,Num,X,Y,Z,Magnitude,Heading");
}

void loop() {
  unsigned long currentTime=millis();
  if (millis() - previousTime >= interval) {
    previousTime += interval; 
    // 接收啟動或停止命令
    if (XBee.available()) {
      char cmd = XBee.read();
      if (cmd == 'y') {
        Serial.println("Start reading...");
        Serial.print("input position");
        //XBee.println("Input x y z (e.g. 100 200 -300):");
        // 等待一行輸入
        while (!XBee.available()) delay(100);
        String input = XBee.readStringUntil('\n');
        input.trim();

        // 嘗試解析三個整數
        int parsed = sscanf(input.c_str(), "%d %d %d", &px, &py, &pz);
        if (parsed == 3) {
          //Serial.print("Reference point: ");
          Serial.print(px); Serial.print(", ");
          Serial.print(py); Serial.print(", ");
          Serial.println(pz);
          //XBee.println("Reference point received.");
          isRunning=true;
        } else {
          //XBee.println("Invalid format. Please send 3 integers like: 100 200 -300");
          isRunning = false;
        }
      } else if (cmd == 'n') {
        isRunning = false;
        Serial.println("Stop reading...");
        XBee.println("Stop reading...");
        num=0;
        return;
      }
    }

    if (!isRunning) return;
    mag.readMag(&x, &y, &z);
    float fx = (x-calx)*0.1;
    float fy = (y-caly)*0.1;
    float fz = (z-calz)*0.1;

    float magnitude = sqrt(fx * fx + fy * fy + fz * fz);
    float heading = atan2(fy, fx) * 180.0 / PI;
    if (heading < 0) heading += 360.0;

    char timeBuffer[10];
    sprintf(timeBuffer, "%02d:%02d:%02d", hour(), minute(), second());

    String data = String(timeBuffer) + "," +String(px) + "," +String(py) + "," +String(pz) + "," + num + "," +
                  String(fx) + "," + String(fy) + "," + String(fz) + "," +
                  String(magnitude, 2) + "," + String(heading, 2);

    Serial.println(data);
    XBee.println(data);

    num++;
  }
}
