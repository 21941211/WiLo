#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H



#include <Arduino.h>
#include "DFRobot_I2C_Multiplexer.h"
#include <Wire.h>
//#include "SapFlow_Driver.h"
#include "config.h"
#include <stdint.h>
#include "payload_Config.h"
#include "defaultWiLoData.h"
#include "RTC_Driver.h"

//Define Register Map Sapflow sensor
#define TEMP_LOW 0x00
#define TEMP_HIGH 0x01
#define HUMID_LOW 0x02
#define HUMID_HIGH 0x03
#define INTERRUPT_DRDY 0x04
#define TEMP_MAX 0x05
#define HUMID_MAX 0x06
#define INTERRUPT_CONFIG 0x07
#define TEMP_OFFSET_ADJUST 0x08
#define HUM_OFFSET_ADJUST 0x09
#define TEMP_THR_L 0x0A
#define TEMP_THR_H 0x0B
#define HUMID_THR_L 0x0C
#define HUMID_THR_H 0x0D
#define CONFIG 0x0E
#define MEASUREMENT_CONFIG 0x0F
#define MID_L 0xFC
#define MID_H 0xFD
#define DEVICE_ID_L 0xFE
#define DEVICE_ID_H 0xFF


// Number of I2C devices
#define DEFAULT_PORT 8 // this is the default port number. The mux has ports 0-7, port 8 is the port on the PCB.

//  Constants for setting sensor mode
#define TEMP_AND_HUMID 0
#define TEMP_ONLY 1
#define HUMID_ONLY 2
#define ACTIVE_LOW 0
#define ACTIVE_HIGH 1
#define LEVEL_MODE 0
#define COMPARATOR_MODE 1

//constants for sensor type addresses
#define I2C_SAPFLOW_ADDR 0x40 // Address for HDC2080 Sapflow sensor
#define I2C_SAPFLOW_ADDR2 0x41 // Address for HDC2080 Sapflow sensor on second port
#define I2C_DENDRO_ADDR 0x68 // Address for HDC2080 Dendrometer sensor
#define I2C_MUX_ADDR 0x70 // Address for I2C Multiplexer


//Constants for sensor type
#define NOT_CONNECTED 0
#define SAPFLOW_SENSOR 1
#define DENDROMETER 2
#define I2C_MUX 3
#define RTC 4



void I2Csetup();
void I2CReadWrite();
void I2C_Mux_SelectPort(uint8_t port);
bool checkPortConnection(uint8_t port);
void passValuesToStruct(uint8_t port, float T1, float T2,uint8_t heatState);
void calculateAverages(uint8_t port);
float calculateHPV(uint8_t port);
bool I2CAddresses(uint8_t *Addresses, uint8_t *AddressesCount);
void initialiseI2CMuxStructs();
void calculateAveragesSFWiloDefault();
void passValuesToStructDefaultWilo( float T1, float T2,uint8_t heatState);

// Struct definition
typedef struct {
    uint8_t addresses[3];
    uint8_t addressCount;
    float sumT1Before;
    float sumT2Before;
    float sumT1During;
    float sumT2During ;
    float sumT1After ;
    float sumT2After; 
    uint16_t countT1Before;
    uint16_t countT2Before ;
    uint16_t countT1During ;
    uint16_t countT2During ;
    uint16_t countT1After ;
    uint16_t countT2After ;
    float avgT1Before;
    float avgT2Before;
    float avgT1During;
    float avgT2During;
    float avgT1After;
    float avgT2After;
    char avgT1Before_buffer[8];
    char avgT2Before_buffer[8];
    char avgT1During_buffer[8];
    char avgT2During_buffer[8];
    char avgT1After_buffer[8];
    char avgT2After_buffer[8];
    char HPV_buffer[8];
    //float HPV;
 char Dendro_Buffer[8];
 float arrDendroRaw[DENDRO_SAMPLE_SIZE]= {0};
 float DendroRawTrimmed ;
 float microns ;
 uint8_t sampleCounterDendro ;
    uint8_t sensorType ;
} I2C_Device_Struct;



// Declare your array or variable externally
extern I2C_Device_Struct i2cDevice[];

#endif // I2C_DRIVER_H

