/*Stellenbosch University Sap-Flow Sensor
* Copyright 2024, Waldo Jordaan, All rights reserved.
* Author: Waldo Jordaan
* ~~~~~~~~~~~~~~~~~~~~~~~~~~Decleration/Acknowledgements/Use~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* This code used herein is heavily inspired and adapted, with my own contribution, from Dotmote Labs and 
* My Circuite (including, but not limited to, David Birds and other contributers acknowledged within)
*
* The code contained herein is for use by the University of Stellenbosch Forestry department.
* This software, the ideas and concepts is protected under Copyright (c). 
* The software, ideas and concepts may be used under Creative Commons License CC BY-NC-SA
* It is highly recommended to contact the author if any doubt remains regarding the use of this property. 
*
* Dotmote Labs: ~ https://dotmotelabs.com/BlogArticles/external-heat-ratio.html
*               ~ https://github.com/dotmote/sapflow/tree/master
* My Circuits:  ~ https://www.youtube.com/watch?v=zoYMU1tA3nI
*               ~ https://drive.google.com/drive/folders/1CN8U-VyOMm5EoAAntuFtYXbBFIAb3Kog
*
* Information from David Birds original code:
* - This software, the ideas and concepts is Copyright (c) David Bird 2018. All rights to this software are reserved.
* 
* - Any redistribution or reproduction of any part or all of the contents in any form is prohibited other than the following:
* - 1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
* - 2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
* - 3. You may not, except with my express written permission, distribute or commercially exploit the content.
* - 4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
*
* - The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
* - software use is visible to an end-user.
* 
* - THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY 
* - OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* - IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
* - FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Node Configuration Constants
#define DEBUG 0
// Variables to store configuration values
int deviceID, gatewayID, sleepTime, sampleRate, timeBeforeHeat, heaterOnTime, heaterOffTime;

// - Wifi config
//const char *ssid = "TJT_Gateway";
char *ssid = nullptr;
const char *password = "22786171";

// SD Card
#include "FS.h"
#include "SD.h"
#include "SPI.h"

bool SD_present = false;

// RTC from Rtc_by_Makuna
//#include <Wire.h> // must be included here so that Arduino library object file references work
//#include <RtcDS3231.h>
//RtcDS3231<TwoWire> Rtc(Wire);
//#define countof(a) (sizeof(a) / sizeof(a[0]))
//RtcDateTime now;
//uint32_t rtcUnixTimestamp;
//char displayDatestring[20];

// Deep sleep
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
//#define TIME_TO_SLEEP 20     /* Time ESP32 will go to sleep (in seconds). 1800 seconds = 30 minutes */
#include "driver/adc.h"

// Wifi
#include <WiFi.h>
//WiFiClient wifiClient;

//#if GATEWAY

#include <ESP32WebServer.h>    //https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <ESPmDNS.h>
#include <HTTPClient.h>

#include "CSS.h" //Includes headers of the web and de style file

/* SERVER */
ESP32WebServer server(80);

/* VARIABLES */
#define servername "TJT" //Define the name to your server... 

//#endif

// temperature sensors
#include <HDC2080.h>

#define ADDR 0x40
#define ADDR2 0x41
HDC2080 sensor(ADDR);
HDC2080 sensor2(ADDR2);

#define HEAT_PIN_SWITCH 26

#define LED_PIN 5

#define BATTERY_PIN 35  // Define the GPIO pin number
#define VOLTAGE_DIVIDER 2.0  // Define the voltage divider factor
#define ADC_RESOLUTION 12  // Define the ADC resolution
#define ADC_MAX 4095  // Maximum value for 12-bit resolution (2^12 - 1)
#define REF_VOLTAGE 3.3  // Reference voltage for ESP32 ADC
#define BATTERY_FULL 4.2  // Full charge voltage
#define BATTERY_EMPTY 3.0  // Discharge cut-off voltage, could actually go down to 2.75 but rather safe.

float batt_voltage;
int batteryPercentage;

unsigned long tempSensorTimer = 0;

//Using millis instead of delay
unsigned long currentMillis;

//Counter for millis since heat pulse was fired
unsigned long millisSinceHeatPulse = 0;
uint8_t heatPulse_state = 0; // 0: before heatpulse; 1: heater on; 2: after heatpulse

//Internal counter for starting/stopping heat pulse
unsigned long millisStartHeatPulse = 0;

//Millis value for last time heater was turned on
unsigned long previousHeaterOnTime = 0;

//Counter for millis since we started tracking reference temperatures
unsigned long millisStartReferenceTemp = 0;

unsigned long millisSinceReferenceTemp = 0;

boolean referenceTempRecorded = false;

float temp1;
float temp2;
float outsideTemp;

long lastWifiReconnectAttempt = 0;

//Counter that will be saved to NVS for keeping track if wake-up was possibly reset btn pushed.
#include <Preferences.h>
Preferences preferences;

// Define the key for storing the variable in NVS
const char* PREF_KEY = "open_gateway";

// Define the default value for the variable
int open_gateway = 0;

String filepath = "";

void enterDeepSleep()
{
  Serial.println(F("Entering deep sleep"));
  //adc_power_off(); found this brakes stuff just before sleep
  open_gateway = 0;
  esp_deep_sleep_start();
}

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    open_gateway++;
    if (open_gateway >= 2){
      open_gateway = 0;
    }
    preferences.putInt(PREF_KEY, open_gateway);
    preferences.end();
    break;
  }
}

void setup()
{

  Serial.begin(115200);
  while (!Serial);

  //Open NVS
  preferences.begin("gateway", false);
  //Read from NVS
  open_gateway = preferences.getInt(PREF_KEY, open_gateway);
  print_wakeup_reason();

  // Serial.print("compiled: ");
  // Serial.print(__DATE__);
  // Serial.println(__TIME__);
  
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Initializing SD card...");
  if (!SD.begin())
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }else{
    Serial.println("Card Mounted Successfully");
    SD_present = true;
  }

  //digitalWrite(LED_PIN, LOW);

  getConfig();

  esp_sleep_enable_timer_wakeup(sleepTime * uS_TO_S_FACTOR);

  // Dynamic allocation of memory for SSID
  ssid = new char[15];  // Allocate enough memory for the string "TJT_Gateway" and a null terminator
  String ssid_name = "TJT_Gateway" + String(gatewayID);
  strcpy(ssid, ssid_name.c_str());  // Assign the value

  
  
  //#if GATEWAY
  if(open_gateway){
    
    doServer_Gateway();
    //digitalWrite(LED_PIN, HIGH);
  }
  else{
  //#else !GATEWAY
    filepath = "/datalog_" + String(deviceID) + ".csv"; //define path to datalog_ID file for Node
    doServer_Node();
    WiFi.disconnect();       // Disconnect from the Wi-Fi network
    //WiFi.mode(WIFI_OFF);    Makes the WiFi_AP not come on at all after restart, needed to remove flash.
  }
  //#endif
  
  checkFileExist(); //check if the datalog file exits, if not initialze first line

  getBatteryVoltage();

  sensorConfig();
}

void loop()
{
  //#if GATEWAY
  //Server
  while(open_gateway==1)//if gateway enabled
  {
    server.handleClient(); //Listen for client connections
  }
  //#endif
  
  doSensing();

  if (millisSinceHeatPulse > (heaterOnTime + heaterOffTime)) //this will start when heat is on i.e. if heat is on for 20s and this is 30s = 10s after heat off
  {
    enterDeepSleep();
  }

}

void getConfig(){
  // Open the file for reading
  File configFile = SD.open("/CONFIG.txt");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return;
  }

  // Read the file into a string
  configFile.readStringUntil('\n');
  String content = configFile.readStringUntil('\n');
  configFile.close();  // Close the file
  
  // Parse the CSV data
  parseConfiguration(content);

  // Output values to verify correct parsing
  Serial.println("Configuration parameters:");
  Serial.println("Device ID: " + String(deviceID));
  Serial.println("Gateway ID: " + String(gatewayID));
  Serial.println("Sleep Time: " + String(sleepTime));
  Serial.println("Sample Rate: " + String(sampleRate));
  Serial.println("Time Before Heat: " + String(timeBeforeHeat));
  Serial.println("Heater On Time: " + String(heaterOnTime));
  Serial.println("Heater Off Time: " + String(heaterOffTime));
}

void parseConfiguration(String data) {
  int idx1 = data.indexOf(',');
  int idx2 = data.indexOf(',', idx1 + 1);
  int idx3 = data.indexOf(',', idx2 + 1);
  int idx4 = data.indexOf(',', idx3 + 1);
  int idx5 = data.indexOf(',', idx4 + 1);
  int idx6 = data.indexOf(',', idx5 + 1);

  // Extract each value from the CSV string
  deviceID = data.substring(0, idx1).toInt();
  gatewayID = data.substring(idx1 + 1, idx2).toInt();
  sleepTime = data.substring(idx2 + 1, idx3).toInt();
  sampleRate = data.substring(idx3 + 1, idx4).toInt();
  timeBeforeHeat = data.substring(idx4 + 1, idx5).toInt();
  heaterOnTime = data.substring(idx5 + 1, idx6).toInt();
  heaterOffTime = data.substring(idx6 + 1).toInt();
}

void doServer_Gateway(){
  WiFi.mode(WIFI_AP);    // Set mode to station
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  //Set your preferred server name, if you use "mcserver" the address would be http://mcserver.local/
  if (!MDNS.begin(servername)) 
  {          
    Serial.println(F("Error setting up MDNS responder!")); 
    ESP.restart(); 
  } 
  
  /*********  Server Commands  **********/
  server.on("/",         SD_dir);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200, "text/plain", "HTTP_POST_RECV");}, handleFileUpload);
  server.on("/esp32log", HTTP_POST, [](){}, handleLocalLog);
  
  server.begin();  
  Serial.println("HTTP server started");
}

void doServer_Node(){
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);    // Set mode to station
  WiFi.begin(ssid, password);

  uint8_t wifi_delay= 0; 
  while (WiFi.status() != WL_CONNECTED)
  {
    if(wifi_delay < 15){
      delay(1000);
      Serial.print(".");
      wifi_delay++;
    }
    else break;
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    SendLocalLog();
  }
  else if(WiFi.status() != WL_CONNECTED){
    Serial.println("");
    Serial.println("WiFi NOT connected");
    Serial.println("Continuing operation without WiFi");
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN, HIGH);
  }
}

void sensorConfig(){
   // Set heat pin as an output
  pinMode(HEAT_PIN_SWITCH, OUTPUT);
  // Set heat pin low to begin with
  digitalWrite(HEAT_PIN_SWITCH, LOW);

  // Initialize I2C communication
  sensor.begin();
  sensor2.begin();

  // Begin with a device reset
  sensor.reset();
  sensor2.reset();

  // Configure Measurements
  sensor.setMeasurementMode(TEMP_AND_HUMID); // Set measurements to temperature and humidity
  sensor2.setMeasurementMode(TEMP_AND_HUMID);
  sensor.setRate(ONE_HZ); // Set measurement frequency to 1 Hz
  sensor2.setRate(ONE_HZ);
  sensor.setTempRes(FOURTEEN_BIT);
  sensor2.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);
  sensor2.setHumidRes(FOURTEEN_BIT);

  //begin measuring
  sensor.triggerMeasurement();
  sensor2.triggerMeasurement();
}

void doSensing(){
  static unsigned long measStartMillis = millis();
  currentMillis = millis() - measStartMillis;

  //start reading temperature for baseline reference check
  if (digitalRead(HEAT_PIN_SWITCH) == LOW && millisStartReferenceTemp == 0)
  {
    Serial.print(F("measStartMillis = "));
    Serial.println(measStartMillis);
    Serial.println(F("Starting to read reference temperatures..."));
    millisStartReferenceTemp = currentMillis;
  }

  if (digitalRead(HEAT_PIN_SWITCH) == LOW && currentMillis - millisStartReferenceTemp >= timeBeforeHeat && previousHeaterOnTime == 0)
  {
    millisStartHeatPulse = currentMillis;
    Serial.println(F("Done reading reference temperatures. Turning heating element on."));
    digitalWrite(HEAT_PIN_SWITCH, HIGH);
    Serial.println(F("Heater ON"));
    previousHeaterOnTime = currentMillis;
    heatPulse_state = 1;
  }

  //turn off heating element after it has been on for >= 10 000 ms -> defined by variable in config.txt
  if (digitalRead(HEAT_PIN_SWITCH) == HIGH && currentMillis - millisStartHeatPulse >= heaterOnTime)
  {
    digitalWrite(HEAT_PIN_SWITCH, LOW);
    Serial.println(F("Heater OFF"));
    heatPulse_state = 2;
  }

  millisSinceHeatPulse = currentMillis - previousHeaterOnTime;
  millisSinceReferenceTemp = currentMillis - millisStartReferenceTemp;

  if (millis() - tempSensorTimer >= sampleRate)
  {
//    now = Rtc.GetDateTime();
//    Serial.print(F("The current time is: "));
//    printDateTime(now);
//    Serial.println();
//    rtcUnixTimestamp = now.Epoch32Time();
//    Serial.print(F("The current unix timestamp is: "));
//    Serial.println(rtcUnixTimestamp);
    digitalWrite(LED_PIN, HIGH);
    temp1 = sensor.readTemp();
    temp2 = sensor2.readTemp();
    Serial.print("Sensor 1 Temperature (C): ");
    Serial.print(temp1);
    Serial.print(" Sensor 2 Temperature (C): ");
    Serial.print(temp2);
    Serial.print("Heat Pulse: ");
    Serial.print(heatPulse_state);
    Serial.print(F(" currentMillis: "));
    Serial.println(currentMillis);
    tempSensorTimer = millis();

    writeMeas();
    digitalWrite(LED_PIN, LOW);
  }
}

void checkFileExist(){
  if (SD.exists(filepath)) {
    Serial.println(F("datalog_ID.txt exists."));
  } else {
    Serial.println(F("datalog_ID.txt does not exist, creating new file..."));
    File file = SD.open(filepath, FILE_APPEND);
    if (!file){
      Serial.println(F("!!! Error creating datalog_ID.csv !!!"));
      return;
    }
    else{
      file.print("currentMillis");
      file.print(",");
      file.print("batt_voltage");
      file.print(",");
      file.print("batteryPercentage");
      file.print(",");
      // write temp1 and temp2 with 10 places following the decimal
      file.print("temp1");
      file.print(",");
      file.print("temp2");
      file.print(",");
      file.println("heatPulse_state");
      file.close();
    }
  }
}

void writeMeas(){
  File file = SD.open(filepath, FILE_APPEND);
  if (!file)
  {
    Serial.println("Error opening datalog_ID.csv for writing");
    return;
  }
  else
  {
    Serial.println("Writing data to /datalog_ID.csv...");
    file.print(currentMillis);
    file.print(",");
    if(millisSinceReferenceTemp == 0){
      file.print(batt_voltage, 2);
    }else{
      file.print("    ");
    }
    file.print(",");
    file.print(batteryPercentage);
    file.print(",");
    // write temp1 and temp2 with 10 places following the decimal
    file.print(temp1, 10);
    file.print(",");
    file.print(temp2, 10);
    file.print(",");
    file.println(heatPulse_state);
    file.close();
    Serial.println("Data written to SD Card");
  }
}

void getBatteryVoltage(){
  int adcValue = analogRead(BATTERY_PIN);  // Read the analog value from battery pin
  batt_voltage = (adcValue / (float)ADC_MAX) * REF_VOLTAGE * VOLTAGE_DIVIDER;  // Calculate the battery voltage
  batteryPercentage = voltageToPercentage(batt_voltage);
  Serial.print("ADC Value: ");
  Serial.print(adcValue);  // Output the raw ADC value
  Serial.print(" - Battery Voltage: ");
  Serial.print(batt_voltage, 2);  // Output the calculated voltage with 2 decimal places
  Serial.print(" - Persentage: ");
  Serial.print(batteryPercentage);
  Serial.println("%");
}

int voltageToPercentage(float voltage) {
  if (voltage >= BATTERY_FULL) {
    return 100;
  } else if (voltage <= BATTERY_EMPTY) {
    return 0;
  } else {
    // Map the voltage range to a percentage
    return (int)((voltage - BATTERY_EMPTY) / (BATTERY_FULL - BATTERY_EMPTY) * 100);
  }
}

File UploadFile;
// Function to handle file uploads to '/esp32log' endpoint
void handleLocalLog() {
    //File UploadFile;
    HTTPUpload& upload = server.upload();  // Get the upload object from the server
    Serial.println(upload.filename);
    Serial.println(upload.currentSize);
    Serial.println(upload.name);
    Serial.println(upload.status);
    Serial.println(upload.type);
    Serial.println(upload.totalSize);

    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if (!filename.startsWith("/")) filename = "/" + filename;  // Ensure the filename starts with '/'
        Serial.print("Upload Start: "); Serial.println(filename);

        // Open the file for writing in SD card (delete if exists)
        SD.remove(filename);  // Remove existing file with the same name
        UploadFile = SD.open(filename, FILE_WRITE);  // Create a new file
        if (!UploadFile) {
            Serial.println("File open failed");
        }
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
        // Write the received bytes to the file
        if (UploadFile) {
            size_t bytesWritten = UploadFile.write(upload.buf, upload.currentSize);
            if (bytesWritten != upload.currentSize) {
                Serial.println("Error during file write");
            }
        }
    } 
    else if (upload.status == UPLOAD_FILE_END) {
        if (UploadFile) {
            // Close the file when the upload is complete
            UploadFile.close();
            Serial.print("Upload Success: "); Serial.println(upload.totalSize);
            server.send(200, "text/plain", "File Uploaded Successfully");
        } else {
            server.send(500, "text/plain", "File Upload Failed");
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
        // If the file upload was aborted
        if (UploadFile) {
            UploadFile.close();
        }
        Serial.println("Upload Aborted");
        server.send(500, "text/plain", "File Upload Aborted");
    }
}

void SendLocalLog(){
  // Open the log file from SD card
  File file = SD.open(filepath, FILE_READ);
  if(!file){
    Serial.println("Error opening /datalog_ID.csv");
    return;
  }
  Serial.println("Sending log file...");
  HTTPClient http;
  http.begin("http://tjt.local/esp32log");
  http.addHeader("Content-Type", "multipart/form-data; boundary=boundary123");

  // Prepare the body of the POST request
  String httpRequestData = "--boundary123\r\n";
  httpRequestData += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filepath + "\"\r\n";
  httpRequestData += "Content-Type: text/csv\r\n\r\n";

  httpRequestData += file.readString();
  file.close();

  httpRequestData += "\r\n--boundary123--\r\n";

  // Send the POST request
  int httpResponseCode = http.POST(httpRequestData);
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println("Response payload: ");
    Serial.println(payload);  // Print the response payload
  } else {
    Serial.print("HTTP Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//void printDateTime(const RtcDateTime &dt)
//{
//  char datestring[20];
//
//  snprintf_P(datestring,
//             countof(datestring),
//             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
//             dt.Month(),
//             dt.Day(),
//             dt.Year(),
//             dt.Hour(),
//             dt.Minute(),
//             dt.Second());
//  Serial.print(datestring);
//}