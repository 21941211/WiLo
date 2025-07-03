#include "debug.h"

void InfiniteStop(){
    Serial.println("Infinite loop");
    while(1);
}

void printSerialDebug(const char *message)
{
    Serial.println(message);
}