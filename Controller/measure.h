#pragma once

#ifndef MEASURE_H
#define MEASURE_H

typedef struct
{
	float temperature;
	float humidity;
	bool bFan; //
	bool bSol[4]; //
				  // 
	uint8_t zoneVolt[8];
	//
	bool fenceLeds[4];
	//
	uint8_t diffPress[8];
	//
	bool diffLeds[4];
	uint8_t lastActiveSole;
}UnitStatus;

extern UnitStatus currStatus;

//extern UnitStatus currStatus;
bool getAverageHumiAndTemp();
void readDiffPressure();
void readFenceVoltage();

void displayTime(); 
void RefreshScreen();
void initLCD();
void clearLCD();

void sensorInit();
double mapdouble(double x, double in_min, double in_max, double out_min, double out_max);

bool checkSunDown();

#endif
