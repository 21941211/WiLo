#include <Arduino.h>
#include <RTClib.h>
#include "config.h"
#include <I2C_Driver.h>




 uint8_t setupRTC();
void printRTCLoop() ;
bool isRTC_DS3231() ;
void testPrintRTC();
String getDateTimeRTC(uint8_t format);


extern RTC_DS3231 rtc;