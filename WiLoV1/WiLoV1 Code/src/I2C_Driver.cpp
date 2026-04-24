#include "I2C_Driver.h"

/*!
 * @file scanI2C.ino
 * @brief Connect I2C devices to I2C Multiplexer, and connect I2C Multiplexer to Arduino, 
 * @n     then download this example open serial monitor to check the I2C addr.
 * @detail  I2C address selection
 * @n       A0  A1  A2  I2C_addr
 * @n       0   0   0   0x70(default)
 * @n       0   0   1   0x71
 * @n       0   1   0   0x72
 * @n       0   1   1   0x73
 * @n       1   0   0   0x74
 * @n       1   0   1   0x75
 * @n       1   1   0   0x76
 * @n       1   1   1   0x77
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      PengKaixing(kaixing.peng@dfrobot.com)
 * @version  V1.0.0
 * @date  2022-03-23
 * @url https://github.com/DFRobot/DFRobot_I2C_Multiplexer
 */
//#include <DFRobot_I2C_Multiplexer.h>


uint8_t muxPossibleAddresses[8] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77};

/*Create an I2C Multiplexer object, the address of I2C Multiplexer is 0x70*/
DFRobot_I2C_Multiplexer I2CMultiplexer(&Wire, 0x70);

/*Plug I2C device onto port 0*/
//uint8_t port = 0;

/*The address of I2C is 0x18*/
uint8_t i2c_addr = 0x40;

/*The register address of I2C*/
uint8_t reg = 0;

/*DENDRO, SF OR I2CMUX*/
uint8_t DeviceAddressesI2C[3]= {0x00, 0x00, 0x00};


bool I2C_MUX_CONNECTED = false; // Set to true if using I2C MUX

// Create an array for the 8 I2C ports


I2C_Device_Struct i2cDevice[9];


void I2C_Mux_SelectPort(uint8_t port){
  I2CMultiplexer.begin();
  I2CMultiplexer.selectPort(port);
}

//Function that scans WiLo I2C addresses and stores them in the i2cDevice struct if found
void I2Csetup(){ 
 Wire.begin(I2C_SDA, I2C_SCL, 400000);
  
  delay(200);
  Serial.println("Scan ready!");

if(I2CAddresses(wilo.i2cAddresses, &wilo.i2cAddressCount)){
  Serial.print(wilo.i2cAddressCount);
 Serial.println(" I2C devices found on default port:");
  for (uint8_t i2cDevIndex = 0; i2cDevIndex < wilo.i2cAddressCount; i2cDevIndex++)
  {
     Serial.print("Test print of address: ");
  Serial.println(wilo.i2cAddresses[i2cDevIndex], HEX);
    switch (wilo.i2cAddresses[i2cDevIndex])
    {
    case 0x40:
        Serial.println("Sapflow sensor found on default port, address 0x40");
        wilo.i2cDeviceType = SAPFLOW_SENSOR;
        wilo.sfconnected=true;
        break;
        case 0x68:
       
        //setupRTC();
        if(isRTC_DS3231()){
        //printRTCLoop();
        wilo.i2cDeviceType=RTC;
        Serial.println("RTC connected on default port");

}else{ 
   Serial.println("Dendrometer found on default port, address 0x68");
  wilo.i2cDeviceType  = DENDROMETER;
        wilo.dendroconnected=true;
}
        
        break;
        case 0x70:
        Serial.print("I2C MUX found on default port, address 0x");
        Serial.println(wilo.i2cAddresses[i2cDevIndex], HEX);
       wilo.i2cDeviceType  = I2C_MUX;
        setMuxConnections(I2C_MUX_CONNECTED_BIT_POS, WILO_SENSOR_CONFIG_BYTE_NUMBER);
        I2C_MUX_CONNECTED = true;
      break;
      default:
      break;
    }
  }
}else{
  Serial.println("No I2C devices found");
}

//I2C_MUX_CONNECTED = true; // Reset the flag before scanning

  //initialiseI2CMuxStructs();

  /*Check if I2C MUX is connected*/
  Wire.beginTransmission(I2C_MUX_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println("I2C MUX found at address: " + String(I2C_MUX_ADDR, HEX));
    I2C_MUX_CONNECTED = true;
  } else {
    Serial.println("No I2C MUX found, using default port");
    I2C_MUX_CONNECTED = false;
  }

if (I2C_MUX_CONNECTED){
   wilo.i2cMuxConnected = true;
        I2CMultiplexer.begin();
        /*Print I2C device of each port of MUX*/
        delay(200);

Serial.println("Scanning I2C MUX ports...");

  for(uint8_t ScannedPort = 0; ScannedPort < 8; ScannedPort++)
  {
     
    Serial.print("Scanning Port:");
    Serial.println(ScannedPort);
    uint8_t *dev = I2CMultiplexer.scan(ScannedPort);
    // uint8_t SensorCount[2] = {0, 0};
    uint8_t SensorCountIndex = 0;
    while (*dev)
    {
      
      Serial.print("  i2c addr 0x");
       // SensorCount[SensorCountIndex] = *dev;
        //Serial.println(*dev, HEX);

        i2cDevice[ScannedPort].addresses[SensorCountIndex] = *dev;
        Serial.println(i2cDevice[ScannedPort].addresses[SensorCountIndex], HEX);
        switch (i2cDevice[ScannedPort].addresses[SensorCountIndex])
        {
        case I2C_SAPFLOW_ADDR:
        setMuxConnections(ScannedPort, I2C_MUX_PORT_CONFIG_BYTE_NUMBER);
          i2cDevice[ScannedPort].sensorType = SAPFLOW_SENSOR;
          Serial.println("Sapflow sensor found on port: " + String(ScannedPort));
          
          break;
          case I2C_SAPFLOW_ADDR2:
          setMuxConnections(ScannedPort, I2C_MUX_PORT_CONFIG_BYTE_NUMBER);
          i2cDevice[ScannedPort].sensorType = SAPFLOW_SENSOR;
          Serial.println("Sapflow sensor found on port: " + String(ScannedPort));
          break;
        case I2C_DENDRO_ADDR:
          if(isRTC_DS3231()){
            i2cDevice[ScannedPort].sensorType = RTC;
            Serial.println("RTC found on port: "+String(ScannedPort));
          }else{
          i2cDevice[ScannedPort].sensorType = DENDROMETER;
          Serial.println("Dendrometer found on port: " + String(ScannedPort));
          setMuxConnections(ScannedPort, I2C_SENSOR_TYPE_CONFIG_BYTE_NUMBER);
          }
          break;
        default:
        Serial.println("No sensor found on port: " + String(ScannedPort));
          i2cDevice[ScannedPort].sensorType = NOT_CONNECTED;
          break;
        }
        SensorCountIndex++;
      dev++;
    }
  }
 
} else{
  Serial.println("I2C MUX not connected, using default port");
}
  
  Serial.println("I2C setup done");

}

bool checkPortConnection(uint8_t port)
{
  if(i2cDevice[port].sensorType > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

uint8_t checkRTC_PortNum(){
  for (uint8_t port = 0; port <8 ; port++){
    if(i2cDevice[port].sensorType == RTC)
    return port;
  } 
  return 9;
}


// Function to initialize the I2C MUX structs
void initialiseI2CMuxStructs(){
  for (uint8_t i = 0; i < 8; i++)
  {
    i2cDevice[i].sensorType = 0;
    i2cDevice[i].addressCount = 0;
    i2cDevice[i].sumT1Before = 0.0f;
    i2cDevice[i].sumT2Before = 0.0f;
    i2cDevice[i].sumT1During = 0.0f;
    i2cDevice[i].sumT2During = 0.0f;
    i2cDevice[i].sumT1After = 0.0f;
    i2cDevice[i].sumT2After = 0.0f;
    i2cDevice[i].countT1Before = 0;
    i2cDevice[i].countT2Before = 0;
    i2cDevice[i].countT1During = 0;
    i2cDevice[i].countT2During = 0;
    i2cDevice[i].countT1After = 0;
    i2cDevice[i].countT2After = 0;
    i2cDevice[i].avgT1Before = 0.0f;
    i2cDevice[i].avgT2Before = 0.0f;
    i2cDevice[i].avgT1During = 0.0f;
    i2cDevice[i].avgT2During = 0.0f;
    i2cDevice[i].avgT1After = 0.0f;
    i2cDevice[i].avgT2After = 0.0f;
    i2cDevice[i].sampleCounterDendro = 0;

}
}

void passValuesToStruct(uint8_t port, float T1, float T2,uint8_t heatState)
{
  switch (heatState)
    {
        case 0:  // Before heating
        i2cDevice[port].sumT1Before += T1;
        i2cDevice[port].sumT2Before += T2;
        i2cDevice[port].countT1Before++;
        i2cDevice[port].countT2Before++;
        break;
    
        case 1: // During heating
        i2cDevice[port].sumT1During += T1;
        i2cDevice[port].sumT2During += T2;
        i2cDevice[port].countT1During++;
        i2cDevice[port].countT2During++;
        break;
    
        case 3: // After heating
        //Serial.println("TEST");
        i2cDevice[port].sumT1After += T1;
        i2cDevice[port].sumT2After += T2;
        i2cDevice[port].countT1After++;
        i2cDevice[port].countT2After++;
        Serial.println("This is a test print to show values being passed AFTER HEAT PULSE to struct for port: " + String(port));
        Serial.print("Sum T1 After: ");
        Serial.println(i2cDevice[port].sumT1After);
        Serial.print("Sum T2 After: ");
        Serial.println(i2cDevice[port].sumT2After);
        Serial.print("Count T1 After: ");
        Serial.println(i2cDevice[port].countT1After);
        Serial.print("Count T2 After: ");
        break;

        default:
        break;
    }
}

void passValuesToStructDefaultWilo( float T1, float T2,uint8_t heatState)
{
  switch (heatState)
    {
        case 0: // Before heating
        wilo.sumT1Before += T1;
        wilo.sumT2Before += T2;
        wilo.countT1Before++;
        wilo.countT2Before++;
        break;

        case 1: // During heating
        wilo.sumT1During += T1;
        wilo.sumT2During += T2;
        wilo.countT1During++;
        wilo.countT2During++;
        break;

        case 3: // After heating
        //Serial.println("TEST");
        wilo.sumT1After += T1;
        wilo.sumT2After += T2;
        wilo.countT1After++;
        wilo.countT2After++;
        break;
    
        default:
        break;
    }
}

void calculateAveragesSFWiloDefault(){
  wilo.SF_T1_Before = wilo.sumT1Before/float(wilo.countT1Before);
  wilo.SF_T2_Before = wilo.sumT2Before/float(wilo.countT2Before);
  wilo.SF_T1_During=wilo.sumT1During/float(wilo.countT1During);
  wilo.SF_T2_During=wilo.sumT2During/float(wilo.countT2During);
  wilo.SF_T1_After=wilo.sumT1After/float(wilo.countT1After);
  wilo.SF_T2_After=wilo.sumT2After/float(wilo.countT2After);

  if (wilo.SF_T1_Before == -40.0f || wilo.SF_T2_Before == -40.0f || wilo.SF_T1_During == -40.0f || wilo.SF_T2_During == -40.0f ||
      wilo.SF_T1_After == -40.0f || wilo.SF_T2_After == -40.0f){
    Serial.println("Sapflow sensor not connected or faulty");
    
    wilo.SF_T1_Before = 1111.11f;
  wilo.SF_T2_Before = 1111.11f;
  wilo.SF_T1_During=1111.11f;
  wilo.SF_T2_During=1111.11f;
  wilo.SF_T1_After=1111.11f;
  wilo.SF_T2_After=1111.11f;
     }
}

void calculateAverages(uint8_t port)
{

    i2cDevice[port].avgT1Before = i2cDevice[port].sumT1Before / float(i2cDevice[port].countT1Before);
    i2cDevice[port].avgT2Before = i2cDevice[port].sumT2Before / float(i2cDevice[port].countT2Before);
    i2cDevice[port].avgT1During = i2cDevice[port].sumT1During / float(i2cDevice[port].countT1During);
    i2cDevice[port].avgT2During = i2cDevice[port].sumT2During / float(i2cDevice[port].countT2During);
    i2cDevice[port].avgT1After = i2cDevice[port].sumT1After / float(i2cDevice[port].countT1After);
    i2cDevice[port].avgT2After = i2cDevice[port].sumT2After / float(i2cDevice[port].countT2After);

   if (i2cDevice[port].avgT1Before == -40.0f || i2cDevice[port].avgT2Before == -40.0f || i2cDevice[port].avgT1During == -40.0f || i2cDevice[port].avgT2During == -40.0f ||
     i2cDevice[port].avgT1After == -40.0f || i2cDevice[port].avgT2After == -40.0f){
    Serial.println("Sapflow sensor not connected or faulty");
    
    i2cDevice[port].avgT1Before = 1111.11f;
    i2cDevice[port].avgT2Before = 1111.11f;
    i2cDevice[port].avgT1During = 1111.11f;
    i2cDevice[port].avgT2During = 1111.11f;
    i2cDevice[port].avgT1After = 1111.11f;
    i2cDevice[port].avgT2After = 1111.11f;
     }

    Serial.print("this is debugging print to show sums, avgs, and counts on port: ");
    Serial.println(port);
    Serial.print("Sum T1 Before: ");
    Serial.println(i2cDevice[port].sumT1Before);
    Serial.print("Sum T2 Before: ");
    Serial.println(i2cDevice[port].sumT2Before);
    Serial.print("Sum T1 During: ");
    Serial.println(i2cDevice[port].sumT1During);
    Serial.print("Sum T2 During: ");
    Serial.println(i2cDevice[port].sumT2During);
    Serial.print("Sum T1 After: ");
    Serial.println(i2cDevice[port].sumT1After);
    Serial.print("Sum T2 After: ");
    Serial.println(i2cDevice[port].sumT2After);
    Serial.print("Count T1 Before: ");
    Serial.println(i2cDevice[port].countT1Before);
    Serial.print("Count T2 Before: ");
    Serial.println(i2cDevice[port].countT2Before);
    Serial.print("Count T1 During: ");
    Serial.println(i2cDevice[port].countT1During);
    Serial.print("Count T2 During: ");
    Serial.println(i2cDevice[port].countT2During);
    Serial.print("Count T1 After: ");
    Serial.println(i2cDevice[port].countT1After);
    Serial.print("Count T2 After: ");
    Serial.println(i2cDevice[port].countT2After);
    Serial.print("Avg T1 Before: ");
    Serial.println(i2cDevice[port].avgT1Before);
    Serial.print("Avg T2 Before: ");
    Serial.println(i2cDevice[port].avgT2Before);
    Serial.print("Avg T1 During: ");
    Serial.println(i2cDevice[port].avgT1During);
    Serial.print("Avg T2 During: ");
    Serial.println(i2cDevice[port].avgT2During);
    Serial.print("Avg T1 After: ");
    Serial.println(i2cDevice[port].avgT1After);
    Serial.print("Avg T2 After: ");
    Serial.println(i2cDevice[port].avgT2After);
    Serial.println();
}



bool I2CAddresses(uint8_t *Addresses, uint8_t *AddressesCount) {
  uint8_t count = 0;

  for (uint8_t address = 10; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
     // Serial.print("I2C device found at address 0x");
     // Serial.print(address, HEX);
      Addresses[count++] = address;
    }
  }
 AddressesCount[0] = count;
  return (count > 0);
}
