#include <Arduino.h>
#include "defaultWiLoData.h"

// #define SDI12_NC 0
// #define SDI12_60CM 1
// #define SDI12_90CM 2
// #define CS655 3

#define I2C_MUX_CONNECTED_BIT_POS 0


#define I2C_MUX_PORT_CONFIG_BYTE_NUMBER 2
#define I2C_SENSOR_TYPE_CONFIG_BYTE_NUMBER 1
#define WILO_SENSOR_CONFIG_BYTE_NUMBER 0

#define SDI12_DD_60 1
#define SDI12_DD_90 2
#define SDI12_CS655 3



void setMuxConnections(uint8_t bitPosition, uint8_t whichByte);
void setSDI12Bits(uint8_t sdi12Type);
void printPayloadConfig();
//void returnPayloadConfig(uint8_t *config, size_t size);
