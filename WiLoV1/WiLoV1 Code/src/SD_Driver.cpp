#include "SD_Driver.h"
#include <vector>
#include <algorithm>

const byte key[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};




// Device ID in HEX
String deviceIDHex;

//uint8_t LoRaKeyRead = 0;
uint8_t endOfFirstPayload = 0;
uint8_t startOfSDI12Payload = 0;
byte dataRead;
uint8_t dataBuffer[];
// std::vector<uint8_t> LoRaBuffer;
// std::vector<uint8_t> PayLoadTest;
//std::vector<uint8_t> wiloConfigBytes;
std::vector<uint8_t> payload;
std::vector<uint8_t> payloadReduced;
uint8_t LoRaBuffer_SDI12[13] = {0};

// Converts NODE_NUMBER to a string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

const char *fileName = DEFAULT_FILE_NAME;
const char *fileName_SDI12;
const char *fileNameMuxDendro = FILE_NAME_I2C_MUX_DENDROMETER;
const char *fileNameMuxSapflow = FILE_NAME_I2C_MUX_SAPFLOW;




void reverseByteOrder(u1_t arr[8]) {
    std::reverse(arr, arr + 8);
}

void parseHexArray(String hexString, uint8_t *outputArray, int length) {
  Serial.print("Parsing hex string: ");
  Serial.println(hexString);

  hexString.trim(); // Remove extra spaces

  if (hexString.length() != length * 2) {
    Serial.println("Error: Hex string length mismatch!");
    return;
  }

  for (int i = 0; i < length; i++) {
    outputArray[i] = strtol(hexString.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
  }

}




bool readParametersFromFile(const char *path) {
  File file = SD.open(path);
  if (!file) {
    Serial.println("Failed to open file.");
    return false;
  }

Serial.println("File opened");
delay(100);
 while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

     if (line.startsWith("APPEUI=")) {
      
      parseHexArray(line.substring(7), APPEUI, 8);
    } else if (line.startsWith("DEVEUI=")) {
      parseHexArray(line.substring(7), DEVEUI, 8);
    } else if (line.startsWith("APPKEY=")) {
      parseHexArray(line.substring(7), APPKEY, 16);
    } else if (line.startsWith("DEVID=")) {
      String deviceIDStr = line.substring(6); // Extract value after "DEVID="
    deviceIDStr.trim(); // Remove any whitespace or newline characters
    Serial.print("Device ID (DEC): ");
    Serial.println(deviceIDStr);
    if (deviceIDStr.length() == 0 || deviceIDStr.toInt() <= 0 || deviceIDStr.toInt() > 255) {
      Serial.println("Error: Invalid or missing DEVID. Using default ID 1.");
      deviceIDStr = "1"; // Default value
    }
    int deviceID = deviceIDStr.toInt();
    deviceIDHex = String(deviceID, HEX);
    if (deviceIDHex.length() < 2) {
      deviceIDHex = "0" + deviceIDHex; // Add leading zero if necessary
    }
    deviceIDHex.toUpperCase();
    Serial.print("Device ID (HEX): ");
    Serial.println(deviceIDHex);
  }
 }

  
  
  file.close();



reverseByteOrder(DEVEUI);

  return true;
}


void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path)
{
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path))
  {
    Serial.println("Dir created");
  }
  else
  {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path)
{
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path))
  {
    Serial.println("Dir removed");
  }
  else
  {
    Serial.println("rmdir failed");
  }
}


void SDSetup(){
  power_SD_ON();
  setSPI(SD_SPI);

  if (!SD.begin(SD_CS_PIN, SPI, 80000000))
  {
    Serial.println("Card Mount Failed");
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
  }
}


String readPayloadConfig(){
  SDSetup();

  Serial.println("Reading payload config from file");

  File file = SD.open(FILE_NAME_PAYLOAD_CONFIG);
  if (!file)
  {
    Serial.println("Failed to open payload config file for reading");
    //return String("Failed to open payload config file for reading");
  }

  String payloadConfig = file.readStringUntil('\n');
  
  Serial.print("Payload Config: ");
  Serial.println(payloadConfig);
  
  file.close();

  power_SD_OFF();

  return payloadConfig;
}

String readLastEntryFromFile(const char *fileName){

  SDSetup();

 
  Serial.printf("Reading last entry from file: %s\n", fileName);

  File file = SD.open(fileName);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return String("Failed to open file for reading");
  }

  // Move to the end of the file
  file.seek(0, SeekEnd);
  
  // Read backwards until we find a newline character
  int pos = file.size() - 1;
  while (pos >= 0 && file.read() != '\n') {
    pos--;
    file.seek(pos);
  }

  // Now read the last line
  String lastLine = file.readStringUntil('\n');
  
  Serial.print("Last entry: ");
  Serial.println(lastLine);
  
  // Optionally, append a new message
  //appendFile(SD, fileName, message);

  file.close();

  power_SD_OFF();

  return lastLine;
}


void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("File renamed");
  }
  else
  {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path))
  {
    Serial.println("File deleted");
  }
  else
  {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char *path)
{
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file)
  {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len)
    {
      size_t toRead = len;
      if (toRead > 512)
      {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  }
  else
  {
    Serial.println("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++)
  {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

void InitialSDSetup()
{
  power_SD_ON();
  setSPI(SD_SPI);
  delay(500);

int LEDFlash = 0;
  while(!SD.begin(SD_CS_PIN, SPI, 80000000)&&LEDFlash<5)
  {
    Serial.println("Card Mount Failed");

    for (size_t i = 0; i < 2; i++)
    {
    digitalWrite( DEBUG_LED_PIN, HIGH);
    delay(20);
    digitalWrite(DEBUG_LED_PIN,LOW);
    delay(20);
    }
    LEDFlash++;
    delay(1000);
    
  }
if (LEDFlash ==5)
{
  Serial.println("SD connect timeout reached");
  goSleep(LIGHT_SLEEP);
}


  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

Serial.println("SD Card mounted successfully");
Serial.println("The following files are available:");
listDir(SD, "/", 0);

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.println("");

    // Read parameters from file

  Serial.println("LoRa key read status:");
  //Serial.println(LoRaKeyRead); 
//if (!LoRaKeyRead){
  Serial.println("Reading loRa keys from file");
  if (!readParametersFromFile(FILE_NAME_SYSPARAMS)) {
    Serial.println("Failed to read parameters from file.");
    pinMode(DEBUG_LED_PIN,OUTPUT);
    while (1){
      digitalWrite(DEBUG_LED_PIN, HIGH);
      delay(3000);
      digitalWrite(DEBUG_LED_PIN, LOW);
    }
//  }
  Serial.println("LoRa keys read successfully");
 // LoRaKeyRead = 1;
}



 
  if (!SD.exists(fileName))
  {
    writeFile(SD, fileName, "Dendrometer,Air Temperature, Air Humidity,Soil temperature,SM orPyro,Battery,SF T1 Before,SF T2 Before,SF T1 During,SF T2 During,SF T1 After,SF T2 After,Boot count\n");
  }

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));


   Serial.println("SD Setup complete");
Serial.println("******************************************************");
}

void writeToSD()
{

  Serial.println("******************************************************");

  SDSetup();

  
  writeToPayloadConfigFile();

  
// if (readLastPayloadConfigBytes(wilo.wiloConfigBytes)) {
//     Serial.println("Decoded bytes:");
//     for (int i = 0; i < 3; ++i) {
//         Serial.println(wilo.wiloConfigBytes[i]);
//     }
// }

   String defaultWiLoData = String(wilo.Dendro) + "," + String(wilo.AT) + "," + String(wilo.RH) + "," + String(wilo.ST)+ "," + String(wilo.SM) 
  + ","+  String(wilo.batt) + "," + String(wilo.SF_T1_Before) + "," + String(wilo.SF_T2_Before) + "," + String(wilo.SF_T1_During) + "," + String(wilo.SF_T2_During)
  + "," + String(wilo.SF_T1_After) + "," + String(wilo.SF_T2_After)  + "," + String(float(wilo.bootCount)/100.0);


  Serial.println("Writing to file: ");
  Serial.println(DEFAULT_FILE_NAME);
  Serial.println("Data:");
  Serial.println(defaultWiLoData);
    WriteToFile(DEFAULT_FILE_NAME, defaultWiLoData.c_str());
    Serial.println("******************************************************");

if(wilo.i2cDeviceType==RTC){
 setupRTC();
 DateTime now = rtc.now();
 char datetimeBuf[30];
snprintf(datetimeBuf, sizeof(datetimeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
         now.year(), now.month(), now.day(),
         now.hour(), now.minute(), now.second());
       
String timeAndBootCount = String(datetimeBuf) + ", Bootcount: " + String(float(wilo.bootCount)/100.0);

 Serial.println("RTC Connected on DEFAULT port, writing to TIME file: "+ timeAndBootCount);
WriteToFile(FILE_NAME_TIME, timeAndBootCount.c_str());
}

for (int port = 0; port <8; port++){
  if(i2cDevice[port].sensorType==RTC){
    setupRTC();
    I2C_Mux_SelectPort(port);
 DateTime now = rtc.now();
 char datetimeBuf[30];
snprintf(datetimeBuf, sizeof(datetimeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
         now.year(), now.month(), now.day(),
         now.hour(), now.minute(), now.second());
       
String timeAndBootCount = String(datetimeBuf) + ", Bootcount: " + String(float(wilo.bootCount)/100.0);

 Serial.println("RTC Connected on port" + String(port) +", writing to TIME file: "+ timeAndBootCount);
WriteToFile(FILE_NAME_TIME, timeAndBootCount.c_str());
  }
}

  power_SD_OFF();

}

void readLastEntry()
{
  #ifdef ENABLE_SD
  SDSetup();

  
  readFile(SD, fileName, 0);
  readFile(SD, fileNameMuxDendro, 0);
  readFile(SD, fileNameMuxSapflow, 0);

  if (SDI12_CONNECTED == 1)
  {
    SDSetup_SDI12();
    readFile(SD, fileName_SDI12, 1);
  }
#endif
  power_SD_OFF();
}

void power_SD_ON()
{
  if(!(digitalRead(SD_ENABLE_PIN)&&digitalRead(LORA_CS_PIN))){
  digitalWrite(SD_ENABLE_PIN, HIGH);
  digitalWrite(LORA_CS_PIN, HIGH); // SET LoRa CS pin HIGH
  delay(300);
  }
}

void power_SD_OFF()
{
  digitalWrite(SD_ENABLE_PIN, LOW);
  digitalWrite(LORA_CS_PIN, LOW); // turn the LED on (HIGH is the voltage level)
  delay(100);
}

void SDSetup_SDI12()
{
  power_SD_ON();
  setSPI(SD_SPI);

  if (!SD.begin(SD_CS_PIN, SPI, 80000000))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  switch (SDI12_TYPE)
  {
  case SDI12_DD_60:
    fileName_SDI12 = FILE_NAME_SDI12_60;
    break;
  case SDI12_DD_90:
    fileName_SDI12 = FILE_NAME_SDI12_90;
    break;
    case SDI12_CS655:
    fileName_SDI12 = FILE_NAME_CS655;
  default:
    break;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  if (!SD.exists(fileName_SDI12)&&SDI12_TYPE == SDI12_DD_60)
  {
    writeFile(SD, fileName_SDI12, "SM1,SM2,SM3,SM4,SM5,SM6,Temp1,Temp2,Temp3,Temp4,Temp5,Temp6\n");
  } else if (!SD.exists(fileName_SDI12)&&SDI12_TYPE == SDI12_DD_90)
  {
    writeFile(SD, fileName_SDI12, "SM1,SM2,SM3,SM4,SM5,SM6,SM7,SM8,SM9,Temp1,Temp2,Temp3,Temp4,Temp5,Temp6,Temp7,Temp8,Temp9\n");
    } else if (!SD.exists(fileName_SDI12) && SDI12_TYPE == SDI12_CS655) {
    writeFile(SD, fileName_SDI12, "SM1,SM2,Temp\n");
    }

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  Serial.println("******************************************************");
}

void writeToSD_SDI12()
{

  SDSetup_SDI12();

String allMeasurements = "";
// if (SDI12_TYPE==SDI12_CS655)
// {
//   allMeasurements = CS655_Measurements_To_String();
// } else{
// allMeasurements = SDI12_Measurements_To_String();
// }

uint8_t buffSizeSDI12 =  getBuffSizeSDI12();

for (int i = 0; i < buffSizeSDI12; i++)
{
  allMeasurements+= String(SDI12_SM[i]) + ",";
}



for (int i = 0; i < buffSizeSDI12; i++)
{
  allMeasurements+= String(SDI12_Temp[i]) + ",";
}

  Serial.println("SDI-12 Measurement check:");
  Serial.println(allMeasurements);
  // appendFile(SD, fileName_SDI12, "\n");
  // appendFile(SD, fileName_SDI12, allMeasurements.c_str());

  WriteToFile(fileName_SDI12,allMeasurements.c_str());
  //appendFile(SD, fileName, ":");

  power_SD_OFF();
}

void parsePayload(const String &payload, uint8_t *byteArray)
{
  int byteIndex = 0; // Index to keep track of current position in byteArray

  // Iterate over the payload, skipping spaces
  for (int i = 0; i < payload.length(); i += 2)
  {
    // Extract two characters at a time
    String byteStr = payload.substring(i, i + 2);

    // Convert the string of two characters into a byte
    byteArray[byteIndex] = strtol(byteStr.c_str(), NULL, 16);

    // Increment the byteIndex
    byteIndex++;
  }
}

void decodePayload(std::vector<uint8_t>& recievedPayload) {
    Serial.println("Decoding payload as seen by TTN");

    Serial.println("Payload size: " + String(recievedPayload.size()) + " Bytes");

    // Serial.println("Payload in HEX:");
    // for (size_t i = 0; i < recievedPayload.size(); i++) {
    //     Serial.print(String(recievedPayload[i], HEX));
    //     Serial.print(" ");
    // }

    Serial.println("\nMeasurements in payload:");
    int i = 0;
    while (i <= recievedPayload.size() - 4) {
        float value = float(recievedPayload[i] * 100.0) + 
                      float(recievedPayload[i + 1]) * 1.0 + 
                      float(recievedPayload[i + 2]) * 0.01;

        Serial.print(value);
        Serial.print(",");
        i += 3;
    }

    Serial.println("");
   Serial.println("Last 3 config bytes of payload in binary:");
for (size_t i = recievedPayload.size() - 3; i < recievedPayload.size(); i++) {
    byte b = recievedPayload[i];
    for (int j = 7; j >= 0; j--) {
        Serial.print((b >> j) & 1);
    }
    Serial.print(" ");
}

    Serial.println();
}



void writeToPayloadConfigFile() {
    SDSetup();

    File file = SD.open(FILE_NAME_PAYLOAD_CONFIG, FILE_WRITE);
    if (file) {
        for (int i = 0; i < sizeof(wilo.wiloConfigBytes); i += 3) {
            for (int j = 0; j < 3; ++j) {
                uint8_t b = (i + j < sizeof(wilo.wiloConfigBytes)) ? wilo.wiloConfigBytes[i + j] : 0;

                for (int bit = 7; bit >= 0; --bit) {
                    file.print((b >> bit) & 1);
                }
            }
            file.println(); // New line after 24 bits
        }

        file.close();
        Serial.println("Payload config written to file (24-bit binary lines).");
    } else {
        Serial.println("Failed to open file for writing!");
    }

    power_SD_OFF();
}

void writeToI2CMuxFiles()
{
  Serial.println("##################################################");
  Serial.print("Writing to I2C file: ");

  // enableSD_ON();
  // setSPI(SD_SPI);
  // delay(500);


//enableSD_ON();

SDSetup();
  setSPI(SD_SPI);
  delay(200);

int LEDFlash = 0;
  while(!SD.begin(SD_CS_PIN, SPI, 80000000)&&LEDFlash<5)
  {
    Serial.println("Card Mount Failed");

    for (size_t i = 0; i < 2; i++)
    {
    digitalWrite( DEBUG_LED_PIN, HIGH);
    delay(20);
    digitalWrite(DEBUG_LED_PIN,LOW);
    delay(20);
    }
    LEDFlash++;
    delay(1000);
    
  }
if (LEDFlash ==5)
{
  Serial.println("SD connect timeout reached");
  goSleep(LIGHT_SLEEP);
}


  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

Serial.println("SD Card mounted successfully");
Serial.println("The following files are available:");
listDir(SD, "/", 0);

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.println("");


  printSerialDebug("Mounting SD card for I2C Mux data writing");


if(!SD.exists(FILE_NAME_I2C_MUX_SAPFLOW))
  {
    Serial.println("I2C mux sapflow data file does not exist, creating it now");
    String SFheader = "";

    for (int i = 0; i < 8; i++)
    {
      for(int j = 0; j < 2; j++)
      {
        SFheader += "P" + String(i) + "T" + String(j + 1) + "B,";
        SFheader += "P" + String(i) + "T" + String(j + 1) + "D,";
        SFheader += "P" + String(i) + "T"+ String(j + 1) + "A,";
      }
    }
    SFheader += "Boot count\n"; // Add boot count at the end
    Serial.println(SFheader);


    writeFile(SD, FILE_NAME_I2C_MUX_SAPFLOW, SFheader.c_str());
  }

if(!SD.exists(FILE_NAME_I2C_MUX_DENDROMETER))
  {
    Serial.println("I2C mux dendro data file does not exist, creating it now");
    String dendroHeader = "";

    for (int i = 0; i < 8; i++)
    {
        dendroHeader += "P" + String(i) + "Dendro(um),";
      }
    

    dendroHeader += "Boot count\n"; // Add boot count at the end
    Serial.println(dendroHeader);
    writeFile(SD, FILE_NAME_I2C_MUX_DENDROMETER, dendroHeader.c_str());
  }

 String i2cMuxSapflowData = "";
  String i2cMuxDendroData = "";
 


  for(int port = 0; port < 8; port++)
  {
   if(i2cDevice[port].sensorType==SAPFLOW_SENSOR)
    {
      Serial.print("Port ");
      Serial.print(port);
      Serial.print(" is connected with SF, writing to file: ");
      Serial.println(FILE_NAME_I2C_MUX_SAPFLOW);
      i2cMuxSapflowData += String(i2cDevice[port].avgT1Before) + ",";
      i2cMuxSapflowData += String(i2cDevice[port].avgT2Before) + ",";
      i2cMuxSapflowData += String(i2cDevice[port].avgT1During) + ",";
      i2cMuxSapflowData += String(i2cDevice[port].avgT2During) + ",";
      i2cMuxSapflowData += String(i2cDevice[port].avgT1After) + ",";
      i2cMuxSapflowData += String(i2cDevice[port].avgT2After) + ",";

      i2cMuxDendroData += "0,";
    }
    else if(i2cDevice[port].sensorType==DENDROMETER)
    {
      Serial.print("Port ");
      Serial.print(port);
      Serial.print(" is connected to dendro, writing to file: ");
      Serial.println(FILE_NAME_I2C_MUX_DENDROMETER);
      i2cMuxDendroData += String(i2cDevice[port].microns) + ",";

      i2cMuxSapflowData += "0,0,0,0,0,0,"; // Add empty data for sapflow
    }
    else
    {
      Serial.print("Port ");
      Serial.print(port);
      Serial.println(" is not connected, writing empty data");
      i2cMuxSapflowData += "0,0,0,0,0,0,"; // Add empty data for sapflow
      i2cMuxDendroData += "0,"; // Add empty data for dendrometer
  }
  }

  i2cMuxSapflowData += String(bootCount) ; // Add boot count at the end
  i2cMuxDendroData += String(bootCount) ; // Add boot count at the end


  Serial.println("Writing I2C Mux data to files");
  Serial.print("I2C Mux Sapflow Data: ");
  Serial.println(i2cMuxSapflowData);
  Serial.print("I2C Mux Dendrometer Data: ");
  Serial.println(i2cMuxDendroData);
  

  WriteToFile(FILE_NAME_I2C_MUX_SAPFLOW, i2cMuxSapflowData.c_str());
  WriteToFile(FILE_NAME_I2C_MUX_DENDROMETER, i2cMuxDendroData.c_str());

}


bool checkBitSet(uint8_t byte, int bitPosition) {
    if (byte & (1 << bitPosition)) {
       return true; // Bit is set
    } else {
        return false; // Bit is not set
    }
}

void reducePayload(uint8_t sdi12Status){
  int payloadPosCounterDendro = 0; // Reset the payload position counter
  int payloadPosCounterSF = 0;

  uint8_t numBytesSDI12 = 0;

  for(int payloadIndex = 0; payloadIndex < 13*3; payloadIndex ++) {
    payloadReduced.push_back(payload[payloadIndex]);
  }

  payloadPosCounterDendro = 13*3; // Update the payload position counter
  payloadPosCounterSF = payloadPosCounterDendro+24;

  if(checkBitSet(payload[payload.size()-3], I2C_MUX_CONNECTED_BIT_POS)) {
      Serial.println("I2C Mux connected");
      for(int port = 0; port < 8; port++) {
        if(checkBitSet(payload[payload.size()-1], port) && checkBitSet(payload[payload.size()-2], port)) {
          Serial.print("Port ");
          Serial.print(port);
          Serial.println(" is connected to a dendrometer, adding data to payloadReduced");  
          for (int bytes = 0; bytes<3; bytes++){
              payloadReduced.push_back(payload[payloadPosCounterDendro]); // Dendrometer data    
              payloadPosCounterDendro++;
              
          }
          payloadPosCounterSF=payloadPosCounterSF+ 18; //18 bytes per sapflow per port
           
      } else if(checkBitSet(payload[payload.size()-1], port)) {
        Serial.print("Port ");
        Serial.print(port);
        Serial.println(" is connected to a sapflow sensor, adding data to payloadReduced");
         for (int i = 0; i < 6; i++) {
          for (int bytes = 0; bytes <3; bytes++){
            payloadReduced.push_back(payload[payloadPosCounterSF]); // Add dendrometer data
            payloadPosCounterSF++;
          }
          }
            payloadPosCounterDendro = payloadPosCounterDendro+3; // 3 Bytes per dendrometer per port
      } else {
        Serial.print("No I2C device connected on port: ");
        payloadPosCounterDendro = payloadPosCounterDendro+3; // 3 Bytes per dnedrometer
        payloadPosCounterSF=payloadPosCounterSF+ 18; //18 bytes per sapflow per port
        Serial.println(port);
      }
    }
       
  } else{
      Serial.println("No MUX connected, going with default");
  }

  if (SDI12_CONNECTED){
  

    Serial.println("SDI12 Connected");
    switch (sdi12Status)
    {
    case SDI12_DD_60:
      numBytesSDI12 = 36;
      break;
        case SDI12_DD_90:
      numBytesSDI12 = 54;
      break;
    
        case SDI12_CS655:
      numBytesSDI12 = 9;
      break;
    
    default:
      break;
    }
     for (int sdi12bytes = payload.size()-numBytesSDI12-3; sdi12bytes < payload.size()-3; sdi12bytes++)
  {
    payloadReduced.push_back(payload[sdi12bytes]);
  }
  }
    for (int lastThreeBytes = payload.size()-3 ; lastThreeBytes < payload.size(); lastThreeBytes++){
      payloadReduced.push_back(payload[lastThreeBytes]);
    }
}

void WriteToFile(const char *fileName, const char *message)
{
  Serial.println("##################################################");
  Serial.print("Writing to file: ");
  Serial.println(fileName);

  power_SD_ON();
  setSPI(SD_SPI);
  delay(200);

  while(!SD.begin(SD_CS_PIN, SPI, 80000000))
  {
    Serial.println("Card Mount Failed");

    for (size_t i = 0; i < 2; i++)
    {
    digitalWrite( DEBUG_LED_PIN, HIGH);
    delay(20);
    digitalWrite(DEBUG_LED_PIN,LOW);
    delay(20);
    }
    delay(1000);
  }

  File file = SD.open(fileName, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }

  if (file.println(message))
  {
    Serial.println("File written successfully");
  }
  else
  {
    Serial.println("Write failed");
  }
  
  file.close();
  
}

void appendStrToPayload(const String &payloadString) {

}


void appendStrToPayload(const std::string &payloadString) {
    std::stringstream ss(payloadString);
    std::string item;

    while (std::getline(ss, item, ',')) {
        float value = std::stof(item);

        int intPart = static_cast<int>(value);
        int thousands = intPart / 100;
        int units = intPart % 100;
        int hundredths = static_cast<int>(std::round((value - intPart) * 100));

        // Clamp values to 0-255 just in case
        uint8_t byte1 = static_cast<uint8_t>(std::min(std::max(thousands, 0), 255));
        uint8_t byte2 = static_cast<uint8_t>(std::min(std::max(units, 0), 255));
        uint8_t byte3 = static_cast<uint8_t>(std::min(std::max(hundredths, 0), 99)); // max should be 99

        payload.push_back(byte1);
        payload.push_back(byte2);
        payload.push_back(byte3);
    }
}

void appendBytesArrToPayload( const uint8_t *bytes, size_t length) {
    for (size_t i = 0; i < length; i ++) {
    payload.push_back(bytes[i]);
    }   
}


bool readLastPayloadConfigBytes(uint8_t *outBytes) {
    SDSetup();

    File file = SD.open(FILE_NAME_PAYLOAD_CONFIG, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading!");
        return false;
    }

    // Seek to end to find the last line
    int fileSize = file.size();
    if (fileSize == 0) {
        Serial.println("File is empty.");
        file.close();
        return false;
    }

    // Read file backwards to find the start of the last line
    int pos = fileSize - 1;
    String lastLine = "";

    // Step backwards to find the last '\n'
    while (pos >= 0) {
        file.seek(pos);
        char c = file.read();
        if (c == '\n') {
            // Start of last line found (unless it's the last character itself)
            if (pos + 1 < fileSize) {
                file.seek(pos + 1);
                lastLine = file.readStringUntil('\n');
            }
            break;
        }
        pos--;
    }

    // If no newline found, the whole file is one line
    if (lastLine == "") {
        file.seek(0);
        lastLine = file.readStringUntil('\n');
    }

    file.close();

    Serial.print("Last line read: ");
    Serial.println(lastLine);

    // Validate length
    if (lastLine.length() < 24) {
        Serial.println("Invalid line (too short).");
        return false;
    }

    // Convert each 8-bit block into a uint8_t
    for (int i = 0; i < 3; ++i) {
        String byteStr = lastLine.substring(i * 8, (i + 1) * 8);
        outBytes[i] = strtol(byteStr.c_str(), nullptr, 2); // Binary to uint8_t
    }

    return true;
}

String getFileNameSDI12(){
  return String(fileName_SDI12);
}