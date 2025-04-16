#include "Arduino.h"
#include "pinmapping.h"
#include "DataProcessing_Driver.h"
#include "debug.h"
#include <vector>

#define SDI12_SOIL_MOISTURE 0
#define SDI12_SALINITY 1
#define SDI12_TEMPERATURE 2
#define SDI12_VOLTAGE 3

#define SDI12_DD_60 0
#define SDI12_DD_90 1
#define SDI12_CS655 2


uint8_t SDI12_Measure(uint8_t measurement);
uint8_t SDI12_Check();
void SDI12_Setup();
void SDI12_TX_RX(String &buffer,String command, int t);
String SDI12_Measurements_To_String();
String CS655_Measurements_To_String();
void SDI12_Shutdown();
void SDI12_TX(String command);
void SDI12_RX(String &buffer);
int testCS655();


// extern double SDI12_SM[12];
// extern double SDI12_Temp[12];
// extern double SDI12_Salinity[12];
// extern double SDI12_SupplyVoltage[1];

extern std::vector<double_t> SDI12_SM;
extern std::vector<double_t> SDI12_Temp;
extern std::vector<double_t> SDI12_Salinity;
extern std::vector<double_t> SDI12_SupplyVoltage;

 //extern uint8_t bufferSizeSDI12;


extern uint8_t SDI12_CONNECTED;
extern uint8_t SDI12_TYPE;
extern uint8_t  SDI12_MEASURE_STATE;
