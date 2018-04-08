#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTPIN1 24     
#define DHTPIN2 26     
#define DHTPIN3 27     
#define DHTPIN4 28     

// Output LEDs definition
const uint8_t WORKLED = 22;

const uint8_t ZONE1FILTCHECK = 38;
const uint8_t ZONE2FILTCHECK = 40;
const uint8_t ZONE3FILTCHECK = 42;
const uint8_t ZONE4FILTCHECK = 44;

const uint8_t ZONE1FENCEFAIL = 39;
const uint8_t ZONE2FENCEFAIL = 41;
const uint8_t ZONE3FENCEFAIL = 43;
const uint8_t ZONE4FENCEFAIL = 45;

// Relay Output definition
const uint8_t SOLENOID1 = 30;
const uint8_t SOLENOID2 = 31;
const uint8_t SOLENOID3 = 32;
const uint8_t SOLENOID4 = 33;

const uint8_t FAN1 = 34;
const uint8_t FAN2 = 35;

// ADC Definition
const uint8_t DIFFCHN1IN = A0;
const uint8_t DIFFCHN1OUT = A1;
const uint8_t DIFFCHN2IN = A2;
const uint8_t DIFFCHN2OUT = A3;
const uint8_t DIFFCHN3IN = A4;
const uint8_t DIFFCHN3OUT = A5;
const uint8_t DIFFCHN4IN = A6;
const uint8_t DIFFCHN4OUT = A7;

// Humidity setpoint 70%
const uint8_t HUMISOLSETP = 70;

// Temperature setpoint, 26 degree
const uint8_t TEMPSOLSETP = 26;

// After all solenoids are activated by any condition, delay time interval
//const uint32_t SOLENOIDCYCLEDELAY = 60000; // 60 secs
const uint32_t SOLENOIDCYCLEDELAY = 10000; // 60 secs

// When Humidity condition is activated, each solenoid working time
//const uint32_t SOLENOIDHUMICYCLE = 30000; // 30 secs
const uint32_t SOLENOIDHUMICYCLE = 5000; // 30 secs

// When Temperature condition is activated, each solenoid working time
//const uint32_t SOLENOIDTEMPCYCLE = 20000; // 20 secs
const uint32_t SOLENOIDTEMPCYCLE = 5000; // 30 secs

// Screen refresh intervals.
const uint32_t TIMEDISPINTERVAL = 1000; // Time display interval 
const uint32_t REFRESHINTERVAL = 2000; // Sensor data display interval
const uint32_t MEASUREINTERVAL = 10000; // DHT22 sensors interval

// Temperature condition keeping time
const uint16_t TEMPDELAY = 1000;

// High temperature setting
const uint8_t HIGHTEMP = 30;

// Low temperature setting
const uint8_t LOWTEMP = 26;

// 
const uint16_t CHECKHOLD = 1000;

// Differnce pressure between in and out pressure sensor
const uint8_t ERRDIFFPRESS = 20; // Unit PSI

// Humi, Temp, Filter voltage stream update interval, 120 sec
const uint32_t STREAMUPDATECYCLE = 120000;

// 
const uint32_t STREAMCHECKCYCLE = 9000;

// Fence voltage linear scaling points
const uint16_t VOLTTOP = 3000;
const uint16_t REALTOP = 12;

// Fence voltage error setting point
const uint8_t ERRORVOLT = 7;

// Fence voltagge error holding time
const uint8_t HOLDINGTIME = 200;

// photocell sensor 
const uint8_t PHOTOCELL = A8;
const uint8_t BRIGHTTHRESHOLD = 100;
const uint16_t PHOTOCELLTIME = 200;

// const uint32_t NIGHTACTIONINTERVAL = 7200 * 1000;
const uint32_t NIGHTACTIONINTERVAL = 300 * 1000;
const uint32_t SOLENOIDNIGHTINTERVAL = 5000;
#endif
