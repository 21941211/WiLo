#include "defaultWiLoData.h"

WiLo_Default_Struct wilo;


void initialiseDefaultWiLoData() {
    wilo.RH = 0.0f;
    wilo.AT = 0.0f;
    wilo.ST = 0.0f;
    wilo.SM = 0.0f;
    wilo.Dendro = 0.0f;
    wilo.batt = 0.0f;
    wilo.bootCount = 0;
    wilo.SF_T1_Before = 0.0f;
    wilo.SF_T2_Before = 0.0f;
    wilo.SF_T1_During = 0.0f;
    wilo.SF_T2_During = 0.0f;
    wilo.SF_T1_After = 0.0f;
    wilo.SF_T2_After = 0.0f;
    wilo.i2cMuxConnected = false;
    wilo.dendroconnected = false;
    wilo.sfconnected = false;
    wilo.sdi12connected = false;
    wilo.arrSampleCounterDendro = 0;
    wilo.DendroRawTrimmed = 0.0f;
    wilo.wiloConfigBytes[0] = 0;
    wilo.wiloConfigBytes[1] = 0;
    wilo.wiloConfigBytes[2] = 0;
    wilo.i2cAddresses[0] = 0;
    wilo.i2cAddresses[1] = 0;
    wilo.i2cAddressCount = 0;

wilo.sumT1Before = 0.0f;
    wilo.sumT2Before = 0.0f;
    wilo.sumT1During = 0.0f;
    wilo.sumT2During  = 0.0f;
    wilo.sumT1After  = 0.0f;
    wilo.sumT2After = 0.0f; 
    wilo.countT1Before = 0.0f;
    wilo.countT2Before  = 0.0f;
    wilo.countT1During  = 0.0f;
    wilo.countT2During = 0.0f;
    wilo.countT1After = 0.0f;
    wilo.countT2After = 0.0f;
    
}
