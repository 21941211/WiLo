#include "RTC_Driver.h"

RTC_DS3231 rtc;
 




uint8_t setupRTC() {
  //Serial.begin(9600);
  Wire.end();
  if(!digitalRead(SF_DENDRO_EN_PIN)){
  pinMode(SF_DENDRO_EN_PIN,OUTPUT);
  digitalWrite(SF_DENDRO_EN_PIN,HIGH);
  delay(100);
  }
 
  Wire.begin(I2C_SDA, I2C_SCL, 400000);

 
uint8_t RTC_PortNumber = checkRTC_PortNum();
if(RTC_PortNumber<8){
  Serial.println("RTC Connected on MUX, Port "+ String(RTC_PortNumber));
  // Check if the DS3231 module is connected

Wire.beginTransmission(I2C_MUX_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println("I2C MUX found at address: " + String(I2C_MUX_ADDR, HEX));
      I2C_Mux_SelectPort(RTC_PortNumber);
  }


  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
   
    //return 0;
    //while (1);
  } else {
    Serial.println("RTC Found!");
  }

 return 1;
  // Uncomment the line below to set the time on the DS3231 module
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
} else {
  Serial.println("No RTC connected on MUX! ");
  return 0;
}
  
}


void testPrintRTC() {
  DateTime now = rtc.now();
 
  Serial.println("Current RTC DateTime:");
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

    //setupRTC();

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

String getDateTimeRTC(uint8_t format) {
  // Format options:
  // 0 = DATE only (YYYY-MM-DD)
  // 1 = TIME only (HH:MM:SS)
  // 2 = DATETIME (YYYY-MM-DD HH:MM:SS)
  
  bool rtcFound = false;
  int rtcPort = -1;
  
  // First check if RTC is on default connection
  if (wilo.i2cDeviceType == RTC) {
   setupRTC();
    rtcFound = true;
  } else {
    // Scan through all MUX ports to find an RTC
    for (int port = 0; port < 8; port++) {
      if (i2cDevice[port].sensorType == RTC) {
       setupRTC();
        I2C_Mux_SelectPort(port);
        rtcFound = true;
        rtcPort = port;
        break;
      }
    }
  }
  
  if (!rtcFound) {
    Serial.println("ERROR: No RTC found on default connection or MUX ports");
    return String("ERROR: No RTC found");
  }
  
  // Get current time from RTC
  DateTime now = rtc.now();
  char buffer[30];
  
  switch(format) {
    case 0: // DATE only
      snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
               now.year(), now.month(), now.day());
      break;
      
    case 1: // TIME only
      snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
               now.hour(), now.minute(), now.second());
      break;
      
    case 2: // DATETIME (default)
    default:
      snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
               now.year(), now.month(), now.day(),
               now.hour(), now.minute(), now.second());
      break;
  }
  
  return String(buffer);
}