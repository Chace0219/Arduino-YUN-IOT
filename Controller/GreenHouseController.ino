  /*
	Author	jinzhouyun
	mail	2435575291@qq.com
	Date	2017.06
*/


#include "GroveStream.h"
#include "Constants.h"
#include "measure.h"

#include <Bridge.h>
#include <Process.h>
#include "FBD.h"
#include <FiniteStateMachine.h>

#include "SIM5216.h"
/* Time Count Variables */
static uint32_t nDisplayRTCTime = millis();
static uint32_t nRefreshTime = millis();
static uint32_t nMeasureTasktime = millis();
static uint32_t nStreamUpdateTime = millis();
static uint32_t nStreamCheckTime = millis();

/* 
	FBD Blocks variables for critical contrl logic 

	Time settings are defined in constant.h, so we don't need to change this.
*/
static TON fanOn, fanOff; // FAN control
static TON filterCheck[4]; // Checking filter pressures
static TON fenceVoltCheck[8]; // Checking fence sensor voltage
static Ftrg fenceVoltFall[8]; 
static Rtrg fenceVoltRise[8];
static Rtrg fenceAlarmTrg[4]; // Checking fence alarm conditions
static Ftrg fenceAlarmDis[4]; // Checking fence alarm conditions


/* FSM Machine Routins for solenoid valve controls */

// Standby Status
void standbyEnter();
void standbyUpdate();

// Solenoide 1 turned on by humi condiftion, Current humidity is smaller than setpoint
void humi1stEnter();
void humi1stUpdate();

// Solenoide 2 turned on by humi condiftion, Current humidity is smaller than setpoint
void humi2ndEnter();
void humi2ndUpdate();

// Solenoide 3 turned on by humi condiftion, Current humidity is smaller than setpoint
void humi3rdEnter();
void humi3rdUpdate();

// Solenoide 4 turned on by humi condiftion, Current humidity is smaller than setpoint
void humi4thEnter();
void humi4thUpdate();

// Solenoide 1 turned on by temperature condiftion, Current temperature is higher than setpoint(example 27 degree)
void temp1stEnter();
void temp1stUpdate();

// Solenoide 2 turned on by temperature condiftion, Current temperature is higher than setpoint(example 27 degree)
void temp2ndEnter();
void temp2ndUpdate();

// Solenoide 3 turned on by temperature condiftion, Current temperature is higher than setpoint(example 27 degree)
void temp3rdEnter();
void temp3rdUpdate();

// Solenoide 4 turned on by temperature condiftion, Current temperature is higher than setpoint(example 27 degree)
void temp4thEnter();
void temp4thUpdate();

// Solenoide delay status after every ones are activeted by any condition 
void delaySoleEnter();
void delaySoleUpdate();

/* State list */
State Standby = State(standbyEnter, standbyUpdate, NULL);
State Humi1st = State(humi1stEnter, humi1stUpdate, NULL);
State Humi2nd = State(humi2ndEnter, humi2ndUpdate, NULL);
State Humi3rd = State(humi3rdEnter, humi3rdUpdate, NULL);
State Humi4th = State(humi4thEnter, humi4thUpdate, NULL);
State Temp1st = State(temp1stEnter, temp1stUpdate, NULL);
State Temp2nd = State(temp2ndEnter, temp2ndUpdate, NULL);
State Temp3rd = State(temp3rdEnter, temp3rdUpdate, NULL);
State Temp4th = State(temp4thEnter, temp4thUpdate, NULL);
State Delay10Min = State(delaySoleEnter, delaySoleUpdate, NULL);

// FSM instance
FSM solenoidFSM = FSM(Standby);

/* Init Variables, */
void InitVar()
{
	currStatus.lastActiveSole = 4;
	currStatus.bFan = 0;

	for (uint8_t idx = 0; idx < 4; idx++)
		currStatus.bSol[idx] = 0;
	currStatus.temperature = 0;

	for (uint8_t idx = 0; idx < 8; idx++)
		currStatus.zoneVolt[idx] = 0;

	for (uint8_t idx = 0; idx < 8; idx++)
		currStatus.diffPress[idx] = 0;

	fanOn.IN = 0;
	fanOn.ET = 0;
	fanOn.PRE = 0;
	fanOn.Q = 0;
	fanOn.PT = TEMPDELAY;

	fanOff.IN = 0;
	fanOff.ET = 0;
	fanOff.PRE = 0;
	fanOff.Q = 0;
	fanOff.PT = TEMPDELAY;

	for (uint8_t idx = 0; idx < 4; idx++)
	{
		filterCheck[idx].IN = 0;
		filterCheck[idx].Q = 0;
		filterCheck[idx].ET = 0;
		filterCheck[idx].PRE = 0;
		filterCheck[idx].PT = CHECKHOLD;
	}

	for (uint8_t idx = 0; idx < 8; idx++)
	{
		fenceVoltCheck[idx].IN = 0;
		fenceVoltCheck[idx].Q = 0;
		fenceVoltCheck[idx].ET = 0;
		fenceVoltCheck[idx].PRE = 0;
		fenceVoltCheck[idx].PT = HOLDINGTIME;

		fenceVoltFall[idx].IN = 0;
		fenceVoltFall[idx].PRE = 0;
		fenceVoltFall[idx].Q = 0;
    
		fenceVoltRise[idx].IN = 0;
		fenceVoltRise[idx].PRE = 0;
		fenceVoltRise[idx].Q = 0;
	}

	for (uint8_t idx = 0; idx < 4; idx++)
	{
		fenceAlarmTrg[idx].IN = 0;
		fenceAlarmTrg[idx].Q = 0;
		fenceAlarmTrg[idx].PRE = 0;

		fenceAlarmDis[idx].IN = 0;
		fenceAlarmDis[idx].Q = 0;
		fenceAlarmDis[idx].PRE = 0;
	}
}

/* Hardware init routine */
void InitPorts()
{
  // Working Led
  pinMode(WORKLED, OUTPUT);

  // Filter Warning Leds
  pinMode(ZONE1FILTCHECK, OUTPUT);
  pinMode(ZONE2FILTCHECK, OUTPUT);
  pinMode(ZONE3FILTCHECK, OUTPUT);
  pinMode(ZONE4FILTCHECK, OUTPUT);

  // Fence Fail Leds
  pinMode(ZONE1FENCEFAIL, OUTPUT);
  pinMode(ZONE2FENCEFAIL, OUTPUT);
  pinMode(ZONE3FENCEFAIL, OUTPUT);
  pinMode(ZONE4FENCEFAIL, OUTPUT);

  // Solenoid Relay outputs 
  pinMode(SOLENOID1, OUTPUT);
  digitalWrite(SOLENOID1, HIGH);
  pinMode(SOLENOID2, OUTPUT);
  digitalWrite(SOLENOID2, HIGH);
  pinMode(SOLENOID3, OUTPUT);
  digitalWrite(SOLENOID3, HIGH);
  pinMode(SOLENOID4, OUTPUT);
  digitalWrite(SOLENOID4, HIGH);

  // FAN Relays
  pinMode(FAN1, OUTPUT);
  pinMode(FAN2, OUTPUT);
}

/* Setup */
void setup()
{
	// Power on delay for LCD and some deveices
	delay(2000);

	// Serial Init
	Serial.begin(115200);
	Serial.println("Program Started!");


	// Bridge starting, we have to set baudrate of Bridge as 115200
	Bridge.begin(115200);
	Serial.println("Bridge Started!");

	delay(2500);

	// Init Routines
	InitVar();
	InitPorts();
	

	// RTC, ADS1115, DHT22, 
	sensorInit();

	// LCD init and Logo Display
	initLCD();

	// Get Humi and temperature form 4 DHT sensors 
	getAverageHumiAndTemp();

	// Read fileter voltages and linear scaling
	readDiffPressure();

	// Read Fence voltage from ADS1115
	readFenceVoltage();

	// LCD Clear
	clearLCD();

	// Display time and Sensor data
	displayTime();
	RefreshScreen();

	/* Upload all current data into stream server */
	temperStream();
	fanStream();
	solenoidStream();
	zoneVoltageStream();

	// Time varaibles proecss for multi process
	nDisplayRTCTime = millis();
	nMeasureTasktime = millis();
}  // end of setup


/* */
void loop()
{
	// Read Fileter Pressure
	readDiffPressure();

	// Read Fence Voltages
	readFenceVoltage();

	// Measure Humi and Temp, and average every specified interval, you can adjust MEASUREINTERVAL in constants.h
	if (nMeasureTasktime + MEASUREINTERVAL < millis())
	{
		// Get temeprature and humidity
		getAverageHumiAndTemp();
		nMeasureTasktime = millis();

		// Display sensor data
		RefreshScreen();
	}

	// FAN control Proc 
	fanOn.IN = currStatus.temperature >= HIGHTEMP;
	TONFunc(&fanOn);
	fanOff.IN = currStatus.temperature < LOWTEMP;
	TONFunc(&fanOff);

	if (fanOn.Q)
	{ // Curent temperature is higher than HIGHTEMP Setting, it will turn on FANS
		if (currStatus.bFan == false)
		{
			currStatus.bFan = true;
			// if fan status is transit, it will send stream
			fanStream();
		}
	}
	if (fanOff.Q)
	{ // Curent temperature is lower than LOWTEMP Setting, it will turn off FANS
		if (currStatus.bFan == true)
		{
			currStatus.bFan = false;
			// if fan status is transit, it will send stream
			fanStream();
		}
	}

	// Filter Voltage Check
	for (uint8_t idx = 0; idx < 4; idx++)
	{
		// Check pressure differnces between in and out sensors.
		filterCheck[idx].IN = abs(currStatus.diffPress[idx * 2] - currStatus.diffPress[idx * 2 + 1]) > ERRDIFFPRESS;
		TONFunc(&filterCheck[idx]);
		// Error present
		currStatus.diffLeds[idx] = filterCheck[idx].Q;
	}
  

	// Fence Voltage Check
	bool bModified = false;
	for (uint8_t idx = 0; idx < 8; idx++)
	{
		fenceVoltCheck[idx].IN = currStatus.zoneVolt[idx] <= ERRORVOLT;
		TONFunc(&fenceVoltCheck[idx]);
		// Check transit voltage status
		fenceVoltFall[idx].IN = fenceVoltCheck[idx].Q;
		FTrgFunc(&fenceVoltFall[idx]);
		fenceVoltRise[idx].IN = fenceVoltCheck[idx].Q;
		RTrgFunc(&fenceVoltRise[idx]);

		// 
		if (fenceVoltFall[idx].Q || fenceVoltRise[idx].Q)
			bModified = true;
	}

	// Check Fence Alarm Condition
	for (uint8_t idx = 0; idx < 4; idx++)
	{
		fenceAlarmTrg[idx].IN = fenceVoltCheck[idx * 2].Q && fenceVoltCheck[idx * 2 + 1].Q;
		RTrgFunc(&fenceAlarmTrg[idx]);
		
		fenceAlarmDis[idx].IN = fenceVoltCheck[idx * 2].Q && fenceVoltCheck[idx * 2 + 1].Q;
		FTrgFunc(&fenceAlarmDis[idx]);
		
		if (fenceAlarmTrg[idx].Q)
		{
			// This line for sending stream when Alarm is activated.
			zoneAlarmStream(idx, 1);
			
			Serial.print(F("Fence "));
			Serial.print(idx + 1);
			Serial.println(F(" Alarm activated!"));
		}

		if (fenceAlarmDis[idx].Q)
		{
			// This line for sending stream when Alarm is deactivated.
			zoneAlarmStream(idx, 0);
			
			Serial.print(F("Fence "));
			Serial.print(idx + 1);
			Serial.println(F(" Alarm deactivated!"));
		}

		if (fenceVoltCheck[idx * 2].Q || fenceVoltCheck[idx * 2 + 1].Q)
			currStatus.fenceLeds[idx] = true;
		else
			currStatus.fenceLeds[idx] = false;
	}

	// If transit in condition in fence voltage status, it will update stream
	if (bModified)
	{ // Update Fence Voltage Stream.
		zoneVoltageStream();
	}

	// FSM module Update
	solenoidFSM.update();

	/* IO Port Action */
	digitalWrite(WORKLED, HIGH);
	digitalWrite(FAN1, currStatus.bFan);
	digitalWrite(FAN2, currStatus.bFan);

	//
	digitalWrite(ZONE1FILTCHECK, currStatus.diffLeds[0]);
	digitalWrite(ZONE2FILTCHECK, currStatus.diffLeds[1]);
	digitalWrite(ZONE3FILTCHECK, currStatus.diffLeds[2]);
	digitalWrite(ZONE4FILTCHECK, currStatus.diffLeds[3]);

	//
	digitalWrite(ZONE1FENCEFAIL, currStatus.fenceLeds[0]);
	digitalWrite(ZONE2FENCEFAIL, currStatus.fenceLeds[1]);
	digitalWrite(ZONE3FENCEFAIL, currStatus.fenceLeds[2]);
	digitalWrite(ZONE4FENCEFAIL, currStatus.fenceLeds[3]);

	// 
	if(currStatus.bSol[0])
		digitalWrite(SOLENOID1, LOW);
	else
		digitalWrite(SOLENOID1, HIGH);

	if (currStatus.bSol[1])
		digitalWrite(SOLENOID2, LOW);
	else
		digitalWrite(SOLENOID2, HIGH);

	if (currStatus.bSol[2])
		digitalWrite(SOLENOID3, LOW);
	else
		digitalWrite(SOLENOID3, HIGH);

	if (currStatus.bSol[3])
		digitalWrite(SOLENOID4, LOW);
	else
		digitalWrite(SOLENOID4, HIGH);

	// TimeDisplay Task
	if (nDisplayRTCTime + TIMEDISPINTERVAL < millis())
	{
		nDisplayRTCTime = millis();
		displayTime();
	}

	// Screen Refresh per specified interval, you can change interval in constants.h
	if (nRefreshTime + REFRESHINTERVAL < millis())
	{
		nRefreshTime = millis();
		RefreshScreen();
	}

	// UpdateStream Temperature and humidity and differnece voltages
	if (nStreamUpdateTime + STREAMUPDATECYCLE < millis())
	{
		Serial.println("I will update roveStream data!");
		temperStream();
		nStreamUpdateTime = millis();
	}

	if (nStreamCheckTime + STREAMCHECKCYCLE < millis())
	{
		nStreamCheckTime = millis();
		procStreamList();
	}
}

/* FSM Procs */
void standbyEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 0;
	solenoidStream();
}

void standbyUpdate()
{

	if (currStatus.humidity < HUMISOLSETP)
	{
		Serial.println("HUMI SETP Condition Issued!");
		switch (currStatus.lastActiveSole)
		{
		case 4:
			solenoidFSM.transitionTo(Humi1st);
			break;
		case 1:
			solenoidFSM.transitionTo(Humi2nd);
			break;
		case 2:
			solenoidFSM.transitionTo(Humi3rd);
			break;
		case 3:
			solenoidFSM.transitionTo(Humi4th);
			break;
		}
	}
	else if (currStatus.temperature > TEMPSOLSETP)
	{
		Serial.println("TEMP SETP Condition Issued!");
		switch (currStatus.lastActiveSole)
		{
		case 4:
			solenoidFSM.transitionTo(Temp1st);
			break;
		case 1:
			solenoidFSM.transitionTo(Temp2nd);
			break;
		case 2:
			solenoidFSM.transitionTo(Temp3rd);
			break;
		case 3:
			solenoidFSM.transitionTo(Temp4th);
			break;
		}
	}
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDCYCLEDELAY)
		{
			Serial.println("Standby Timing!");
			currStatus.lastActiveSole = 4;

		}
		// Solenoid Turn off all
		for (uint8_t idx = 0; idx < 4; idx++)
			currStatus.bSol[idx] = 0;
	}
}

void humi1stEnter()
{
	currStatus.bSol[0] = 1;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 0;
	solenoidStream();
}

void humi1stUpdate()
{
	currStatus.lastActiveSole = 1;
	//Serial.println("HUMI1 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Standby);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDHUMICYCLE)
		{
			if (currStatus.humidity < HUMISOLSETP)
				solenoidFSM.transitionTo(Humi2nd);
			else if (currStatus.humidity > TEMPSOLSETP)
				solenoidFSM.transitionTo(Temp2nd);
		}
	}
}

void humi2ndEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 1;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 0;
	solenoidStream();
}

void humi2ndUpdate()
{

	currStatus.lastActiveSole = 2;
	//Serial.println("HUMI2 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Standby);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDHUMICYCLE)
		{
			if (currStatus.humidity < HUMISOLSETP)
				solenoidFSM.transitionTo(Humi3rd);
			else if (currStatus.humidity > TEMPSOLSETP)
				solenoidFSM.transitionTo(Temp3rd);
		}
	}

}

void humi3rdEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 1;
	currStatus.bSol[3] = 0;
	solenoidStream();
}

void humi3rdUpdate()
{
	currStatus.lastActiveSole = 3;
	//Serial.println("HUMI3 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Standby);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDHUMICYCLE)
		{
			if (currStatus.humidity < HUMISOLSETP)
				solenoidFSM.transitionTo(Humi4th);
			else if (currStatus.humidity > TEMPSOLSETP)
				solenoidFSM.transitionTo(Temp4th);
		}
	}
}

void humi4thEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 1;

	solenoidStream();
}

void humi4thUpdate()
{

	currStatus.lastActiveSole = 4;
	//Serial.println("HUMI4 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Delay10Min);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDHUMICYCLE)
			solenoidFSM.transitionTo(Delay10Min);
	}
}

void temp1stEnter()
{
	currStatus.bSol[0] = 1;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 0;

	solenoidStream();
}

void temp1stUpdate()
{
	currStatus.lastActiveSole = 1;
	//Serial.println("TEMP1 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Standby);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDTEMPCYCLE)
		{
			if (currStatus.humidity < HUMISOLSETP)
				solenoidFSM.transitionTo(Humi2nd);
			else if (currStatus.humidity > TEMPSOLSETP)
				solenoidFSM.transitionTo(Temp2nd);
		}
	}
}

void temp2ndEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 1;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 0;

	solenoidStream();
}

void temp2ndUpdate()
{

	currStatus.lastActiveSole = 2;
	//Serial.println("TEMP2 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Standby);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDTEMPCYCLE)
		{
			if (currStatus.humidity < HUMISOLSETP)
				solenoidFSM.transitionTo(Humi3rd);
			else if (currStatus.humidity > TEMPSOLSETP)
				solenoidFSM.transitionTo(Temp3rd);
		}
	}
}

void temp3rdEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 1;
	currStatus.bSol[3] = 0;

	solenoidStream();
}

void temp3rdUpdate()
{
	currStatus.lastActiveSole = 3;
	//Serial.println("TEMP3 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Standby);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDTEMPCYCLE)
		{
			if (currStatus.humidity < HUMISOLSETP)
				solenoidFSM.transitionTo(Humi4th);
			else if (currStatus.humidity > TEMPSOLSETP)
				solenoidFSM.transitionTo(Temp4th);
		}
	}

}

void temp4thEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 1;

	solenoidStream();
}

void temp4thUpdate()
{
	currStatus.lastActiveSole = 4;
	//Serial.println("TEMP4 status!");

	if (currStatus.temperature <= TEMPSOLSETP && currStatus.humidity >= HUMISOLSETP)
		solenoidFSM.transitionTo(Delay10Min);
	else
	{
		if (solenoidFSM.timeInCurrentState() > SOLENOIDHUMICYCLE)
			solenoidFSM.transitionTo(Delay10Min);
	}
}

void delaySoleEnter()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 0;
	solenoidStream();
}

void delaySoleUpdate()
{
	currStatus.bSol[0] = 0;
	currStatus.bSol[1] = 0;
	currStatus.bSol[2] = 0;
	currStatus.bSol[3] = 0;
	currStatus.lastActiveSole = 4;
	//Serial.println("Delay status!");

	if (solenoidFSM.timeInCurrentState() > SOLENOIDCYCLEDELAY)
	{
		if (currStatus.humidity < HUMISOLSETP)
			solenoidFSM.transitionTo(Humi1st);
		else if (currStatus.temperature > TEMPSOLSETP)
			solenoidFSM.transitionTo(Temp1st);
		else
			solenoidFSM.transitionTo(Standby);
	}
}
