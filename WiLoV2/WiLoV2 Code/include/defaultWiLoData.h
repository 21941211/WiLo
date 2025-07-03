#ifndef DEFAULT_WILO_DATA_H
#define DEFAULT_WILO_DATA_H

#include <Arduino.h>
#include <stdint.h>
#include "config.h"



void initialiseDefaultWiLoData();

// Struct definition
struct WiLo_Default_Struct {
    float RH;
    float AT;
    float ST;
    float SM;
    float Dendro;
    float batt;
    int bootCount;
    float SF_T1_Before;
    float SF_T2_Before;
    float SF_T1_During;
    float SF_T2_During;
    float SF_T1_After;
    float SF_T2_After;
    bool i2cMuxConnected;
    bool dendroconnected;
    bool sfconnected;
    bool sdi12connected;
    uint8_t i2cDeviceType;
    float arrDendroRaw[DENDRO_SAMPLE_SIZE];
    int arrSampleCounterDendro;
    float DendroRawTrimmed;
    uint8_t wiloConfigBytes[3];
    uint8_t i2cAddresses[2];
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
uint8_t i2cAddressCount;};
   




// Declare your array or variable externally
extern WiLo_Default_Struct wilo;

#endif // DEFAULT_WILO_DATA_H