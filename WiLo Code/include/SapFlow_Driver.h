#include <HDC2080.h>
#include <Wire.h>
#include "pinmapping.h"
#include <stdlib.h>
#include <vector>
#include "MeasureFlags.h"
#include "SDI-12_Driver.h"
#include <cmath>
#include "debug.h"

#define ADDR 0x40
#define ADDR2 0x41

void SFSetup();
void SF_Measure();
void testRead();
float calculateHPV(const std::vector<float>& arrSapflowT1, const std::vector<float>& arrSapflowT2, uint8_t StartHP, uint8_t endHP) ;



 extern float avgT1Before;
  extern float avgT2Before ;
  extern float avgT1During ;
  extern float avgT2During ;
  extern float avgT1After ;
  extern float avgT2After ;
  extern float HPV ;
  

  extern uint8_t HEATER_STATE ;