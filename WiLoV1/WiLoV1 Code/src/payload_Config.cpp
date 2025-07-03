#include "payload_Config.h"


//uint8_t payloadConfig[3] = {0}; // Initialize payloadConfig to 0

void setMuxConnections(uint8_t bitPosition, uint8_t whichByte){
    wilo.wiloConfigBytes[whichByte] |= (1 << bitPosition);
}


void setSDI12Bits(uint8_t sdi12Type){
    wilo.wiloConfigBytes[WILO_SENSOR_CONFIG_BYTE_NUMBER] |= sdi12Type<<1;
    // Serial.print("Setting SDI-12 type bits to: ");
    // Serial.print(sdi12Type, BIN);
    // Serial.print(" in byte ");
    // Serial.print(WILO_SENSOR_CONFIG_BYTE_NUMBER);
    // Serial.print(". New value: ");
    // Serial.println(payloadConfig[WILO_SENSOR_CONFIG_BYTE_NUMBER], BIN);
    // Serial.print("Full payloadConfig: ");
    // for (int i = 0; i < 3; i++) {
    //     // Print with leading zeros for clarity (optional formatting)
    //     String binStr = String(payloadConfig[i], BIN);
    //     while (binStr.length() < 8) binStr = "0" + binStr;
    //     Serial.print(binStr);
    //     Serial.print(" ");
    // }
    // Serial.println();
}

void printPayloadConfig() {
    Serial.print("Payload Config Bytes: ");
    for (int i = 0; i < 3; i++) {
        Serial.print(wilo.wiloConfigBytes[i], BIN);
        Serial.print(" ");
    }
    Serial.println();
}