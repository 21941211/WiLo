#ifndef SD_DRIVER_H
#define SD_DRIVER_H

#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "config.h"
#include "SPI_Driver.h"
#include "DHT22_Driver.h"
#include "SoilMoisture_Driver.h"
#include "LinPotDendro_Driver.h"
#include "BatteryLevel_Driver.h"
#include "DS18B20_Driver.h"
#include "DataProcessing_Driver.h"
#include "SDI-12_Driver.h"
#include "Lora_Driver.h"
#include "SapFlow_Driver.h"
#include "debug.h"
#include "I2C_Driver.h"
#include "payload_Config.h"
#include "RTC_Driver.h"



#define BUFFERSIZE 41

extern byte dataRead;
extern const char* fileName;
extern uint8_t dataBuffer[BUFFERSIZE];
// extern std::vector<uint8_t> LoRaBuffer;
// extern std::vector<uint8_t> PayLoadTest;
// extern std::vector<uint8_t> PayLoadMuxTest;
extern std::vector<uint8_t> payload;
extern std::vector<uint8_t> payloadReduced;
extern uint8_t LoRaBuffer_SDI12[13];
//extern uint8_t LoRaKeyRead;


void InitialSDSetup();
void SDSetup_SDI12();
void writeToSD();
void writeToSD_SDI12();
void readFile(fs::FS &fs, const char * path,uint8_t NormalOrSDI12);
void readLastEntry();
void power_SD_ON();
void power_SD_OFF();
void decodePayload(std::vector<uint8_t>& recievedPayload);
void writeToI2CMuxFiles();
void WriteToFile(const char *fileName, const char *message);
String readLastEntryFromFile(const char *fileName);
void appendStrToPayload(const std::string &payloadString);
void appendBytesArrToPayload( const uint8_t *bytes, size_t length);
void writeToPayloadConfigFile() ;
bool readLastPayloadConfigBytes(uint8_t *outBytes);
void reducePayload(uint8_t sdi12Status);
String getFileNameSDI12();
void writeToMuxFile(int port, uint8_t sensorType, const char *message);
void createFileBasic(const char* fileName, const char* header);

#endif