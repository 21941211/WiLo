#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "pinmapping.h"
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


#define BUFFERSIZE 41

extern byte dataRead;
extern const char* fileName;
extern uint8_t dataBuffer[BUFFERSIZE];
extern std::vector<uint8_t> LoRaBuffer;
extern std::vector<uint8_t> PayLoadTest;
extern uint8_t LoRaBuffer_SDI12[13];



void SDSetup();
void SDSetup_SDI12();
void writeToSD();
void writeToSD_SDI12();
void readFile(fs::FS &fs, const char * path,uint8_t NormalOrSDI12);
void readLastEntry();
void enableSD_ON();
void enableSD_OFF();
void decodePayload ();