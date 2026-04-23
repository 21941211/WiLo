
#include "SD_Driver.h"
#include <HDC2080.h>
#include <Wire.h>
#include "config.h"
#include <stdlib.h>
#include <vector>
#include "MeasureFlags.h"
#include "SDI-12_Driver.h"
#include <cmath>
#include "debug.h"
#include "I2C_Driver.h"



#define ADDR 0x40
#define ADDR2 0x41

#define BEFORE_HEAT 0
#define DURING_HEAT 1
#define WAIT_AFTER_HEAT 2
#define AFTER_HEAT 3

void SFSetup();
void SF_Measure();
void SFtestRead();

//float calculateHPV(const std::vector<float>& arrSapflowT1, const std::vector<float>& arrSapflowT2, uint8_t StartHP, uint8_t endHP) ;




//  extern float avgT1Before;
//   extern float avgT2Before ;
//   extern float avgT1During ;
//   extern float avgT2During ;
//   extern float avgT1After ;
//   extern float avgT2After ;
//   extern float HPV ;
  

  extern uint8_t HEATER_STATE ;