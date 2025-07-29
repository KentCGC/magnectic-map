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
  
  Serial.println("Time,Num,X,Y,Z,Magnitude,heading");
  XBee.println("Time,Num,X,Y,Z,Magnitude,heading");
}

void loop() {
  qmc.read(&x, &y, &z,&azimuth);

  float fx = x;
  float fy = y;
  float fz = z;
  float magnitude = sqrt(fx * fx + fy * fy + fz * fz);
  float heading =azimuth;
  int mag5 = find_nearest_multiple_of_5(magnitude);

  char timeBuffer[10];
  sprintf(timeBuffer, "%02d:%02d:%02d", hour(), minute(), second());

  char xBuf[6], yBuf[6], zBuf[6];
  formatFourDigit(x, xBuf);
  formatFourDigit(y, yBuf);
  formatFourDigit(z, zBuf);

  String data = String(timeBuffer) + "," + num + "," +
                String(xBuf) + "," + String(yBuf) + "," + String(zBuf) + "," + magnitude+","+heading;

  Serial.println(data);
  XBee.println(data);

  num++;
  delay(500);
}

void formatFourDigit(int val, char* buffer) {
  char sign = (val >= 0) ? '+' : '-';
  val = abs(val);
  sprintf(buffer, "%c%04d", sign, val);  
}

int find_nearest_multiple_of_5(float number) {
  float remainder = fmod(number, 5.0);
  if (remainder == 0.0)
    return (int)number;
  else if (remainder < 2.5)
    return (int)(number - remainder);
  else
    return (int)(number + (5.0 - remainder));
}
