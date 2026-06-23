#include "LoRa_Driver.h"

using namespace std;

uint8_t mydata[] = "00002360055620238100230860024460244402462024530251902487000001";

float num = 12.45;

uint8_t newNum[] = "00000000255000628000250000099200600002736002727002750002733002793002766005940";


uint8_t wiloConfigBytes[3] = {0, 0, 0}; // Initialize payloadConfig to 0

static osjob_t sendjob;

RTC_DATA_ATTR unsigned int LoRaTX_Complete = 0;
uint8_t NoJoin = 0;




#ifdef ENABLE_SD
 u1_t APPEUI[8];
 u1_t DEVEUI[8];
 u1_t APPKEY[16];
 #endif

void LoRaSetup()
{

 #ifdef ENABLE_SD
    //readLastEntry();
    String defaultWiLoLast = readLastEntryFromFile(DEFAULT_FILE_NAME);
    String defaultWiLoMuxDendro = readLastEntryFromFile(FILE_NAME_I2C_MUX_DENDROMETER);
    String defaultWiLoMuxSapflow = readLastEntryFromFile(FILE_NAME_I2C_MUX_SAPFLOW);
    String SDI12 = "";
    
    Serial.println("******************************************************");
    Serial.println("Default WiLo Data from SD Card:");
    Serial.println(defaultWiLoLast);
   Serial.println("******************************************************");
    int dendroLastComma = defaultWiLoMuxDendro.lastIndexOf(',');
    if (dendroLastComma > 0) defaultWiLoMuxDendro = defaultWiLoMuxDendro.substring(0, dendroLastComma);
    Serial.println(defaultWiLoMuxDendro);
  Serial.println("******************************************************");
    Serial.println("Default WiLo Mux Sapflow Data from SD Card:");
    int sapflowLastComma = defaultWiLoMuxSapflow.lastIndexOf(',');
    if (sapflowLastComma > 0) defaultWiLoMuxSapflow = defaultWiLoMuxSapflow.substring(0, sapflowLastComma);
    Serial.println(defaultWiLoMuxSapflow);
   Serial.println("******************************************************");

appendStrToPayload(std::string(defaultWiLoLast.c_str()));
appendStrToPayload(std::string(defaultWiLoMuxDendro.c_str()));
appendStrToPayload(std::string(defaultWiLoMuxSapflow.c_str()));

    if(SDI12_CONNECTED){

    Serial.println("SDI12 data from SD Card:");


        switch (SDI12_TYPE)
        {
        case SDI12_DD_60:
            Serial.println("60 CM drill and drop connected");
            SDI12 = readLastEntryFromFile(FILE_NAME_SDI12_60);
            break;
             case SDI12_DD_90:
             Serial.println("90 CM drill and drop connected");
            SDI12 = readLastEntryFromFile(FILE_NAME_SDI12_90);
            break;
             case SDI12_CS655:
             Serial.println("CS655 connected");
            SDI12 = readLastEntryFromFile(FILE_NAME_CS655);
            break;
        
        default:
            break;
        }
        SDI12 = SDI12.substring(0,SDI12.length()-1);
    Serial.println(SDI12.c_str());
   appendStrToPayload(std::string(SDI12.c_str())) ;
   Serial.println("******************************************************");
     }

  //uint8_t decoded[3];
if (readLastPayloadConfigBytes(wilo.wiloConfigBytes)) {
    Serial.println("Decoded bytes:");
    for (int i = 0; i < 3; ++i) {
        Serial.println(wilo.wiloConfigBytes[i]);
    }
}

appendBytesArrToPayload(wilo.wiloConfigBytes, sizeof(wilo.wiloConfigBytes));

reducePayload(SDI12_TYPE);
Serial.println("******************************************************");
Serial.println("Decoding standard payload: ");
decodePayload(payload);
Serial.println("******************************************************");
Serial.println("Decoding reduced payload: ");
decodePayload(payloadReduced);
Serial.println("******************************************************");


 #else
    Serial.println("SD Card Disabled, using default LoRa parameters");

 #endif

Serial.println("Setting up LoRa");

    setSPI(LORA_SPI);

    // LMIC init
    os_init();
     delay(200);
    Serial.println("OS Init Complete");
    //  Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
     delay(200);
    Serial.println("LMIC_reset complete");
    //  Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
     delay(200);
    Serial.println("do_send complete");
    Serial.println("******************************************************");
}

void printHex2(unsigned v)
{
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void onEvent(ev_t ev)
{
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev)
    {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;
    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;
    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;
    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        {
            u4_t netid = 0;
            devaddr_t devaddr = 0;
            u1_t nwkKey[16];
            u1_t artKey[16];
            LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
            Serial.print("netid: ");
            Serial.println(netid, DEC);
            Serial.print("devaddr: ");
            Serial.println(devaddr, HEX);
            Serial.print("AppSKey: ");
            for (size_t i = 0; i < sizeof(artKey); ++i)
            {
                if (i != 0)
                    Serial.print("-");
                printHex2(artKey[i]);
            }
            Serial.println("");
            Serial.print("NwkSKey: ");
            for (size_t i = 0; i < sizeof(nwkKey); ++i)
            {
                if (i != 0)
                    Serial.print("-");
                printHex2(nwkKey[i]);
            }
            Serial.println();
        }
        // Disable link check validation (automatically enabled
        // during join, but because slow data rates change max TX
        // size, we don't use it in this example.
        LMIC_setLinkCheckMode(0);
        break;
    /*
    || This event is defined but not used in the code. No
    || point in wasting codespace on it.
    ||
    || case EV_RFU1:
    ||     Serial.println(F("EV_RFU1"));
    ||     break;
    */
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        break;
    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;
    case EV_TXCOMPLETE:
        LoRaTX_Complete = 1;
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
        if (LMIC.dataLen)
        {
            Serial.print("Received ");
            Serial.print(LMIC.dataLen);
            Serial.println(" bytes of payload");
        }
        // Schedule next transmission
        // os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
        MEASURE_COMPLETE = 0;
        //LoRaKeyRead = 0;
        LMIC_shutdown();
        delay(1000);

        if (SDI12_CONNECTED)
        {
            goSleep(WILO_MAX_CYCLE_TIME_WITH_SDI12_CONNECTED);
        }
        else
#ifdef ENABLE_LORA_TEST
 goSleep(DEEP_SLEEP_LORA_TEST);
#else
            goSleep(WILO_MAX_CYCLE_TIME);
#endif
        break;
    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;
    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;
    case EV_RXCOMPLETE:
        // data received in ping slot
        Serial.println(F("EV_RXCOMPLETE"));
        break;
    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;
    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;
    /*
    || This event is defined but not used in the code. No
    || point in wasting codespace on it.
    ||
    || case EV_SCAN_FOUND:
    ||    Serial.println(F("EV_SCAN_FOUND"));
    ||    break;
    */
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_TXCANCELED:
        Serial.println(F("EV_TXCANCELED"));
        break;
    case EV_RXSTART:
        /* do not print anything -- it wrecks timing */
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
       // LoRaKeyRead = 0;

        LMIC_shutdown();
        delay(1000);
        goSleep(LIGHT_SLEEP);
        break;

    default:
        Serial.print(F("Unknown event: "));
        Serial.println((unsigned)ev);
        break;
    }
}

void do_send(osjob_t *j)
{

    // setSPI(LORA_SPI);
    //  Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND)
    {
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else
    {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, payloadReduced.data(), payloadReduced.size(), 0); // dataBuffer
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}


void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = LORA_CS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST_PIN,
    .dio = {LORA_DIO0_PIN, LORA_DIO1_PIN, LMIC_UNUSED_PIN},
};

void appendConfigToPayload(){
    for (int i = 0; i < 3; i++)
    {
       payload.resize(i+1);
       payload[i] = wiloConfigBytes[i];

    }
}

