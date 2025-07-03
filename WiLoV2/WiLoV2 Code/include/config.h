
//LoRa Pins
#define LORA_SCK_PIN 36
#define LORA_MISO_PIN 37
#define LORA_MOSI_PIN 35
#define LORA_CS_PIN 34
#define LORA_RST_PIN 18
#define LORA_DIO0_PIN 33
#define LORA_DIO1_PIN 38
#define LORA_SPI 1

//File names
#define DEFAULT_FILE_NAME "/Measurements.csv"
#define FILE_NAME_SDI12_60 "/SDI_12_60cm_Measurements.csv"
#define FILE_NAME_SDI12_90 "/SDI_12_90cm_Measurements.csv"
#define FILE_NAME_CS655 "/SDI_12_CS655_Measurements.csv"
#define FILE_NAME_I2C_MUX_SAPFLOW "/I2C_Mux_Sapflow_Measurements.csv"
#define FILE_NAME_I2C_MUX_DENDROMETER "/I2C_Mux_Dendrometer_Measurements.csv"
#define FILE_NAME_SYSPARAMS "/parameters.txt"
#define FILE_NAME_PAYLOAD_CONFIG "/PayloadConfig.txt"
#define FILE_NAME_TIME "/Time.csv"


//DHT22 and SM enable pin
#define DHT22_SM_ENABLE_PIN 11


//Soil moistures sensor pins
#define SOIL_MOISTURE_PIN 9//1


// DHT22 Pins
#define DHTPIN 8


// Temperature sensor pins
#define DS1B20_PIN 6

// Battery level detector pins
#define BATTERY_ENABLE_PIN 3
#define BATTERY_ADC_PIN 1

//Sapflow pins
#define HEAT_PIN_SWITCH 11
#define SF_DENDRO_EN_PIN GPIO_NUM_2

// I2C Pins
#define I2C_SDA 17
#define I2C_SCL 21



//Dendrometer pins
//#define DENDROMETER_PIN 8
//#define DENDROMETER_ENABLE_PIN GPIO_NUM_2//4


//Onboard LED pin
#define DEBUG_LED_PIN 15

//SDI-12 drill and drop sensor pins
#define SDI12_EN_PIN 7
#define SDI12_DATA_PIN 5

//SD card defines
#define SD_SCK_PIN  36
#define SD_MISO_PIN  37
#define SD_MOSI_PIN  35
#define SD_CS_PIN    16
#define SD_SPI 2

#define SD_ENABLE_PIN 4

// Number of samples taken for each dendrometer
#define DENDRO_SAMPLE_SIZE 10


//Sapflow sensor heat pulse state measuring time
#define SAMPLE_TIME_BEFORE_HP 5000 //5 seconds
#define SAMPLE_TIME_DURING_HP 10000 // 10 seconds
#define SAMPLE_TIME_AFTER_HP 10000 // 10 seconds

//Deep sleep times
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define WILO_MAX_CYCLE_TIME  1200      /* Maximum cycle time of the WiLo in seconds. Wilo is active for time x, deep sleep is thus WILO_MAX_CYCLE_TIME-x */
#define DEEP_SLEEP_LORA_TEST  120      /* Time ESP32 will go to sleep (in seconds) */
#define WILO_MAX_CYCLE_TIME_WITH_SDI12_CONNECTED  1200     /* Maximum cycle time of the WiLo in seconds, if SDI12 is connected. Wilo is active for time x, deep sleep is thus WILO_MAX_CYCLE_TIME-x */
#define LIGHT_SLEEP 5 /*When a transmission fails, WILO goes into light sleep for 5 seconds*/
#define LORA_CONNECT_TIMEOUT 60 /*During this time, the system tries to reconnect to LoRa gateway after a failed transmission attempt. If timeout is reached. WiLo goes into deep sleep*/


//WiFi Timeout
#define WIFI_TIMEOUT 15000 // Timeout for WiFi connection in milliseconds