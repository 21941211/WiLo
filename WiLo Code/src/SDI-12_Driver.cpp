#include "SDI-12_Driver.h"
#include <SDI12.h>
#include "driver/rtc_io.h"
#include "DeepSleep_Driver.h"

 RTC_DATA_ATTR uint8_t SDI12_CONNECTED = 0;
 RTC_DATA_ATTR uint8_t SDI12_TYPE = 3;

 uint32_t SDI12_DATA_REQUEST_DELAY;
   uint8_t bufferSizeSDI12;

unsigned long currentMillis_SDI12;
unsigned long previousMillis_SDI12 = 0;

//CS655 variables
unsigned long previousMillisC = 0;
unsigned long previousMillisD = 0;
bool measurementRequested = false;
bool dataRequesting = false;
bool dataReceiving = false;

uint8_t MeasurementRequest1 = 0;
uint8_t MeasurementRequest2 = 0;
uint8_t MeasurementRequest3 = 0;

unsigned long timeSinceRequest = 0;
unsigned long requestTime = 0;
unsigned long metaDataRecievedTime = 0;
unsigned long timeSinceMetaDataReceived = 0;
unsigned long timeSinceMetaDataReceivedTime = 0;

uint8_t SM_MEASUREMENT_REQUESTED = 0;
uint8_t SM_DATA_REQUESTED = 0;
uint8_t ST_MEASUREMENT_REQUESTED = 0;
uint8_t ST_DATA_REQUESTED = 0;
uint8_t SDI12_SM_DONE = 0;
uint8_t SDI12_ST_DONE = 0;
uint8_t LAST_REQUEST;
uint8_t METADATA_RECEIVED = 0;
uint8_t SDI12_MEASURE_STATE = 0;

char     sensorAddress = '6'; /*!< The address of the SDI-12 sensor */
String sdiResponse = "";




#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MEASUREMENT 0
#define DATA 1
#define DATA 2

struct StringMeasurements
{
  String Address;
  String soilMoisture;
  String Salinity;
  String Temperature;
  String Voltage;
  String CS655;
};

struct MetaData
{
  String soilMoisture;
  String Salinity;
  String Temperature;
  String Voltage;
  String CS655;
};

/** Define the SDI-12 bus */
SDI12 mySDI12(SDI12_DATA_PIN);

String myCommand = "";

StringMeasurements stringMeasurements; // Declare an instance of the struct to hold the measurements

MetaData measurementsMeta;

// double SDI12_SM[12];
// double SDI12_Temp[12];
// double SDI12_Salinity[12];
// double SDI12_SupplyVoltage[1];

std::vector<double_t> SDI12_SM;
std::vector<double_t> SDI12_Temp;
std::vector<double_t> SDI12_Salinity;
std::vector<double_t> SDI12_SupplyVoltage;
std::vector<double_t> CS655;

int SDI12_90cm_Delay = 2000;
int SDI112_120cm_Delay = 3000;

void SDI12_Setup()
{



  pinMode(SDI12_EN_PIN, OUTPUT);
  digitalWrite(SDI12_EN_PIN, HIGH);
  delay(500);


  rtc_gpio_hold_dis(GPIO_NUM_5);
  gpio_reset_pin(GPIO_NUM_5);


  Serial.println("SDI12 is now ON");
   Serial.println("******************************************************");
  Serial.println("Opening SDI-12 bus...");
  mySDI12.begin(GPIO_NUM_5);

delay(500);
  int datPin = mySDI12.getDataPin();
Serial.print("Data pin: ");
Serial.println(datPin);
 // delay(600); // DO NOT CHANGE OR REMOVE!
}

uint8_t SDI12_Check()
{

  //mySDI12.begin();
  //delay(500); // DO NOT CHANGE OR REMOVE!
  stringMeasurements.Address = "";

  SDI12_TX_RX(stringMeasurements.Address, "?!", 300);
  if (stringMeasurements.Address[0] == 0)
  {
    Serial.println("SDI-12 Device not connected");
    Serial.println("******************************************************");
    return 0;
  }
  else
  {
    Serial.println("SDI-12 Device connected");
    Serial.print("Device Address: ");
    Serial.println(stringMeasurements.Address[0]);
    switch (stringMeasurements.Address[0])
    {
    case 'J':
      SDI12_TYPE = SDI12_DD_60;
      SDI12_DATA_REQUEST_DELAY = 1000;
      bufferSizeSDI12 = 6;
      Serial.println("Device is SENTEK DRILL & DROP 60cm");
      
      break;
    case 'S':
      SDI12_TYPE = SDI12_DD_90;
      SDI12_DATA_REQUEST_DELAY = 2500;
      bufferSizeSDI12 = 9;
      Serial.println("Device is SENTEK DRILL & DROP 90cm");
      break;
      case '6':
      SDI12_TYPE = SDI12_CS655;
       SDI12_DATA_REQUEST_DELAY = 2500;
      bufferSizeSDI12 = 3;
      CS655.resize(bufferSizeSDI12);
      SDI12_MEASURE_STATE = 2;
      Serial.println("Device is CS655");
      break;
    default:
      break;
    }
    SDI12_SM.resize(bufferSizeSDI12);
    SDI12_Temp.resize(bufferSizeSDI12);

   

    Serial.print("SDI-12 data request delay: ");
    Serial.println(SDI12_DATA_REQUEST_DELAY);

    Serial.print("SDI-12 buffer size: ");
    Serial.println(SDI12_SM.size());

//InfiniteStop();
   // mySDI12.clearBuffer();
  // mySDI12.end();
    Serial.println("******************************************************");
  }

  return 1;
}

uint8_t SDI12_Measure(uint8_t measurement)
{

  String commands[] = {"C!", "C1!", "C2!", "C9!", "D0!", "D1!", "M!"};
  String measurementNames[] = {"Soil Moisture", "Salinity", "Temperature", "Supply Voltage"};
  String metaData = "";
  String receivedData = ""; // Initialize an empty string for received data

  if (stringMeasurements.Address[0]=='6')
  {
    
  }
  



  if (!SM_MEASUREMENT_REQUESTED)
  {
    Serial.println("Requesting SDI-12 " + measurementNames[measurement] + " Measurement");
     Serial.println("******************************************************");
    SDI12_TX(stringMeasurements.Address[0] + commands[measurement]);
    SM_MEASUREMENT_REQUESTED = 1;
    LAST_REQUEST = MEASUREMENT;
    requestTime = millis();
    timeSinceRequest = 0;
    return 0;
  }

  if (!METADATA_RECEIVED)
  {
    if ((LAST_REQUEST == MEASUREMENT) && (timeSinceRequest >= 100))
    {
      switch (measurement)
      {
      case SDI12_SOIL_MOISTURE:
        SDI12_RX(measurementsMeta.soilMoisture);
        Serial.println("Soil Moisture Metadata: ");
        Serial.println(measurementsMeta.soilMoisture);
        break;
      case SDI12_TEMPERATURE:
        SDI12_RX(measurementsMeta.Temperature);
        Serial.println("Temperature Metadata: ");
        Serial.println(measurementsMeta.Temperature);
        break;
      default:
        break;
      }

      Serial.println("");
      Serial.println("Metadata received");
       Serial.println("******************************************************");
      METADATA_RECEIVED = 1;
      timeSinceMetaDataReceivedTime = 0;
      metaDataRecievedTime = millis();
      return 0;
    }
    else
    {

      timeSinceRequest = millis() - requestTime;
      return 0;
    }
  }

  if (METADATA_RECEIVED)
  {
    if (!SM_DATA_REQUESTED && timeSinceMetaDataReceived >= SDI12_DATA_REQUEST_DELAY)
    {

      Serial.println("Requesting SDI-12 " + measurementNames[measurement] + " Data");
       Serial.println("******************************************************");
      SDI12_TX(stringMeasurements.Address[0] + commands[4]);
      SM_DATA_REQUESTED = 1;
      LAST_REQUEST = DATA;
      requestTime = millis();
      timeSinceRequest = 0;

      return 0;
    }
    else if (!SM_DATA_REQUESTED)
    {
      timeSinceMetaDataReceived = millis() - metaDataRecievedTime;
      return 0;
    }

    if ((LAST_REQUEST == DATA) && (timeSinceRequest >= SDI12_DATA_REQUEST_DELAY))
    {
      // Serial.print("Available data: ");
      // Serial.println(mySDI12.available());
      // Serial.println("Data available!");
    
      switch (measurement)
      {
      case SDI12_SOIL_MOISTURE:
        SDI12_RX(stringMeasurements.soilMoisture);

        if (stringMeasurements.Address[0] == 'S')
        {
         // delay(300);
          Serial.println("Requesting second set of data");
          SDI12_TX_RX(stringMeasurements.soilMoisture, stringMeasurements.Address[0] + commands[5], 100);
        }
      
        Serial.print("Recieved data buffer: ");
        Serial.println(stringMeasurements.soilMoisture);
        extractValuesFromStringSDI12(stringMeasurements.soilMoisture, SDI12_SM.data(), stringMeasurements.Address, bufferSizeSDI12);
       

        // Print the extracted values
       Serial.println("Extracted SDI-12 Soil Moisture Values:");

        // Serial.println(stringMeasurements.soilMoisture);


        for (size_t i = 0; i < bufferSizeSDI12; ++i)
        {
          Serial.print("Value ");
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.println(SDI12_SM[i], 6);
        }
 Serial.println("******************************************************");
        break;
      case SDI12_TEMPERATURE:
        SDI12_RX(stringMeasurements.Temperature);
         if (stringMeasurements.Address[0] == 'S')
        {
          //delay(300);
            Serial.println("Requesting second set of data");
          SDI12_TX_RX(stringMeasurements.Temperature, stringMeasurements.Address[0] + commands[5], 100);
        }
        Serial.print("Recieved data buffer: ");
        Serial.println(stringMeasurements.Temperature);
        extractValuesFromStringSDI12(stringMeasurements.Temperature, SDI12_Temp.data(), stringMeasurements.Address, bufferSizeSDI12);
       

 Serial.println("Extracted SDI-12 Soil Temperature Values:");

 // Serial.println(stringMeasurements.Temperature);
        // Print the extracted values
        for (size_t i = 0; i < bufferSizeSDI12; ++i)
        {
          Serial.print("Value ");
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.println(SDI12_Temp[i], 5);
        }
        Serial.println("******************************************************");
        break;
      default:
        break;
      }

      SM_DATA_REQUESTED = 0;
      SM_MEASUREMENT_REQUESTED = 0;
      METADATA_RECEIVED = 0;
      timeSinceMetaDataReceived = 0;
      timeSinceMetaDataReceivedTime = 0;
      return 1;
    }
    else
    {

      // Serial.println("Waiting for data");
      timeSinceRequest = millis() - requestTime;
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

void SDI12_TX(String command)
{
  mySDI12.sendCommand(command);
}

void SDI12_RX(String &buffer)
{

  while (mySDI12.available())
  {
    char c = mySDI12.read();
   // Serial.write(c); // write the response to the screen
   if (c != '\n' && c != '\r' && c != '\0')
   {
        buffer += c;     // Append the character to the receivedData string
   }
   
  }
  // Serial.println("");
}

void SDI12_TX_RX(String &buffer, String command, int t)
{
  SDI12_TX(command);
  delay(t);
  SDI12_RX(buffer);
}

String SDI12_Measurements_To_String()
{
  String measurements = "";
  char buffer[8]; // Buffer to hold formatted values

  for (size_t i = 0; i < bufferSizeSDI12; ++i)
  {
    sprintf(buffer, "%07.2f", SDI12_SM[i]); // Ensure at least 6 characters with 2 decimal places
    measurements += String(buffer);
    measurements += ",";
  }

  for (size_t i = 0; i < bufferSizeSDI12; ++i)
  {
    sprintf(buffer, "%07.2f", SDI12_Temp[i]);
    measurements += String(buffer);
    measurements += ",";
  }

  // Remove the last comma
  measurements = measurements.substring(0, measurements.length() - 1);

  return measurements;
}

String CS655_Measurements_To_String()
{
  String measurements = "";
  char buffer[8]; // Buffer to hold formatted values

  for (size_t i = 0; i < bufferSizeSDI12; ++i)
  {
    sprintf(buffer, "%07.2f", CS655[i]); // Ensure at least 6 characters with 2 decimal places
    measurements += String(buffer);
    measurements += ",";
  }

  // Remove the last comma
  measurements = measurements.substring(0, measurements.length() - 1);

  return measurements;
}


void SDI12_Shutdown()
{

mySDI12.end();
Serial.println("Closing SDI-12 bus...");

  rtc_gpio_hold_dis(GPIO_NUM_5);
  pinMode(GPIO_NUM_5, OUTPUT);
  digitalWrite(GPIO_NUM_5, LOW);
  digitalWrite(SDI12_EN_PIN, LOW);
  rtc_gpio_hold_en(GPIO_NUM_5); // Lock GPIO 5 state

  Serial.println("SDI12 is now OFF");

  Serial.println("******************************************************");
  // delay(5000);

  // Serial.println("Data pulled low, going to sleep now:");

  // goSleep(LIGHT_SLEEP);
}

// void testCS655(){
// // first command to take a measurement
//   myCommand = String(sensorAddress) + "C!";
//   Serial.println(myCommand);  // echo command to terminal

//   mySDI12.sendCommand(myCommand);
//   delay(30);  // wait a while for a response

//   while (mySDI12.available()) {  // build response string
//     char c = mySDI12.read();
//     if ((c != '\n') && (c != '\r')) {
//       sdiResponse += c;
//       delay(10);  // 1 character ~ 7.5ms
//     }
//   }
//   if (sdiResponse.length() > 1)
//     Serial.println(sdiResponse);  // write the response to the screen
//   mySDI12.clearBuffer();


//   delay(2500);       // delay between taking reading and requesting data
//   sdiResponse = "";  // clear the response string


//   // next command to request data from last measurement
//   myCommand = String(sensorAddress) + "D0!";
//   Serial.println(myCommand);  // echo command to terminal

//   mySDI12.sendCommand(myCommand);
//   delay(30);  // wait a while for a response

//   while (mySDI12.available()) {  // build string from response
//     char c = mySDI12.read();
//     if ((c != '\n') && (c != '\r')) {
//       sdiResponse += c;
//       delay(10);  // 1 character ~ 7.5ms
//     }
//   }
//   if (sdiResponse.length() > 1)
//     Serial.println(sdiResponse);  // write the response to the screen
//   mySDI12.clearBuffer();

// }


int testCS655() {
  unsigned long currentMillis = millis();

  if (!measurementRequested) {
    // First command to take a measurement
    String myCommand = String(sensorAddress) + "C!";
    Serial.println(myCommand);  // Echo command to terminal
    mySDI12.sendCommand(myCommand);
      previousMillisC = currentMillis; // Store time of request
    delay(30);
    measurementRequested = true;
    return 0;
  }

currentMillis = millis();

  if (measurementRequested && (currentMillis - previousMillisC >= 30)&&!dataRequesting) {
    // Read response after 30ms
    String sdiResponse = "";
    while (mySDI12.available()) {  // Build response string
      char c = mySDI12.read();
      if ((c != '\n') && (c != '\r') && c != ('\0')) {
        measurementsMeta.CS655 += c;
        delay(10);
      }
    }
    if (measurementsMeta.CS655.length() > 1)
      Serial.println("CS655 Metadata: "+ measurementsMeta.CS655);  // Write the response to the screen
    mySDI12.clearBuffer();
    previousMillisD = currentMillis; // Start delay for data request
    //measurementRequested = false;
    dataRequesting = true;
    return 0;
  }

  if (dataRequesting && (currentMillis - previousMillisD >= 2500)&&!dataReceiving) {
    // Next command to request data from last measurement
    String myCommand = String(sensorAddress) + "D0!";
    Serial.println(myCommand);  // Echo command to terminal
    mySDI12.sendCommand(myCommand);
    previousMillisC = currentMillis; // Store time of request
    delay(30);
    dataRequesting = true;
    dataReceiving = true;
    return 0;
  }

  if (dataReceiving && (currentMillis - previousMillisC >= 30)) {
    // Read response after 30ms
    String sdiResponse = "";
    while (mySDI12.available()) {  // Build response string
      char c = mySDI12.read();
      if ((c != '\n') && (c != '\r') && (c != '\0')) {
        stringMeasurements.CS655 += c;
        delay(10);
      }
    }
    if (stringMeasurements.CS655.length() > 1) {
      Serial.println("CS655 Data" + stringMeasurements.CS655);  // Write the response to the screen
      extractValuesFromStringSDI12(stringMeasurements.CS655,CS655.data(),stringMeasurements.Address,bufferSizeSDI12);
      mySDI12.clearBuffer();
      Serial.println("Extracted CSS Soil Moisture (V1 and V1) amd temperature (V3) Values:");

        // Serial.println(stringMeasurements.soilMoisture);


        for (size_t i = 0; i < bufferSizeSDI12; ++i)
        {
          Serial.print("Value ");
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.println(CS655[i], 6);
        }
 Serial.println("******************************************************");
      dataReceiving = false;
      return 1; // Data recorded successfully
    }
  }

  return 0; // No data recorded yet
}
