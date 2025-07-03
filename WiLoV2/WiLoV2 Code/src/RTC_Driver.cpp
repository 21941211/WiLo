#include "RTC_Driver.h"

RTC_DS3231 rtc;
 




uint8_t setupRTC() {
  //Serial.begin(9600);
  Wire.end();
  if(!digitalRead(SF_DENDRO_EN_PIN)){
  pinMode(SF_DENDRO_EN_PIN,OUTPUT);
  digitalWrite(SF_DENDRO_EN_PIN,HIGH);
  }
  delay(100);
  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  delay(100);
 
  // Check if the DS3231 module is connected
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    return 0;
    //while (1);
  }
 return 1;
  // Uncomment the line below to set the time on the DS3231 module
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}


void printRTCLoop() {
    while(1){
  DateTime now = rtc.now();
 
  // Print the current date and time
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
 
  delay(1000);
    }
}

void setRTCFromSerial() {
  Serial.println("Enter date and time: YYYY-MM-DD HH:MM:SS");

  while (!Serial.available()) {
    delay(100);
  }

  String input = Serial.readStringUntil('\n');
  Serial.print("Received: ");
  Serial.println(input);

  int year, month, day, hour, minute, second;
  if (sscanf(input.c_str(), "%d-%d-%d %d:%d:%d", 
             &year, &month, &day, &hour, &minute, &second) == 6) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.println("RTC updated successfully.");
  } else {
    Serial.println("Invalid format! Use: YYYY-MM-DD HH:MM:SS");
  }
}


bool isRTC_DS3231() {

    setupRTC();

  Wire.beginTransmission(0x68);
  Wire.write(0x0F); // DS3231 status register
  if (Wire.endTransmission() != 0) {
    return false; // No device or failed to write register
  }

  Wire.requestFrom(0x68, 1);
  if (Wire.available()) {
    uint8_t status = Wire.read();
    // DS3231's status register will have reasonable values like 0x00 to 0xFF
    // You could refine this by checking known status bits, e.g. bit 7 (OSF) is 0/1.
    return true;
  }

  return false;
}