
#include <Bridge.h>
#include <Process.h>

/* Groove Stream Relevant Variables */

void solenoidStream();
void updateGroveStreams(String queryString);
void temperStream();
void zoneVoltageStream();
void fanStream();
void zoneAlarmStream(uint8_t idxZone, uint8_t nValue);
void addStreamList(String QueryString);
void procStreamList();
void initLCD();
