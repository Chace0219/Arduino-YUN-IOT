#include "Constants.h"
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <LiquidCrystal_I2C.h>

#include "measure.h"
#include "RTClib.h"
#include "DHT.h"

LiquidCrystal_I2C lcd01(0x3F, 20, 4);
LiquidCrystal_I2C lcd02(0x27, 20, 4);

RTC_DS1307 rtc;

Adafruit_ADS1115 ads1(0x48);
Adafruit_ADS1115 ads2(0x49);
Adafruit_ADS1115 ads3(0x4B);
Adafruit_ADS1115 ads4(0x4A);

/* Object instants */
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);
DHT dht4(DHTPIN4, DHTTYPE);

static UnitStatus currStatus;

bool getAverageHumiAndTemp()
{
	Serial.println(F("I will get data from DHT22 sensors!"));
	float temp = 0.0;
	float humi = 0.0;

	Serial.print(F("1st temp is "));
	Serial.print(dht1.readTemperature(), 1);
	Serial.println(F("C."));
	temp += dht2.readTemperature();

	Serial.print(F("1st humi is "));
	Serial.print(dht1.readHumidity(), 1);
	Serial.println(F("%."));
	humi += dht1.readHumidity();

	Serial.print(F("2nd temp is "));
	Serial.print(dht2.readTemperature(), 1);
	Serial.println(F("C."));
	temp += dht2.readTemperature();

	Serial.print(F("2nd humi is "));
	Serial.print(dht2.readHumidity(), 1);
	Serial.println(F("%."));
	humi += dht2.readHumidity();

	// 
	Serial.print(F("3rd temp is "));
	Serial.print(dht3.readTemperature(), 1);
	Serial.println(F("C."));
	temp += dht3.readTemperature();

	Serial.print(F("3rd humi is "));
	Serial.print(dht3.readHumidity(), 1);
	Serial.println(F("%."));
	humi += dht3.readHumidity();

	// 
	Serial.print(F("4th temp is "));
	Serial.print(dht4.readTemperature(), 1);
	Serial.println(F("C."));
	temp += dht4.readTemperature();

	Serial.print(F("4th humi is "));
	Serial.print(dht4.readHumidity(), 1);
	Serial.println(F("%."));
	humi += dht4.readHumidity();

	currStatus.temperature = temp / 4;
	currStatus.humidity = humi / 4;

	Serial.print(F("Average temperature is "));
	Serial.println(currStatus.temperature, 1);

	Serial.print(F("Average humidity is "));
	Serial.println(currStatus.humidity, 1);
  
}

void sensorInit()
{
	Wire.begin();
	// 
	if (!rtc.begin())
	{
		Serial.println("Couldn't find RTC");
		while (1);
	}

	if (!rtc.isrunning())
	{
		Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
	}

	ads1.begin();
	ads2.begin();
	ads3.begin();
	ads4.begin();

	/* I2C sensors Init */
	dht1.begin();
	dht2.begin();
	dht3.begin();
	dht4.begin();
}

void readDiffPressure()
{
	uint16_t nAdcVal = analogRead(DIFFCHN1IN);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[0] = map(nAdcVal, 5, 45, 0, 100);

	nAdcVal = analogRead(DIFFCHN1OUT);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[1] = map(nAdcVal, 5, 45, 0, 100);

	nAdcVal = analogRead(DIFFCHN2IN);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[2] = map(nAdcVal, 5, 45, 0, 100);

	nAdcVal = analogRead(DIFFCHN2OUT);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[3] = map(nAdcVal, 5, 45, 0, 100);

	nAdcVal = analogRead(DIFFCHN3IN);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[4] = map(nAdcVal, 5, 45, 0, 100);

	nAdcVal = analogRead(DIFFCHN3OUT);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[5] = map(nAdcVal, 5, 45, 0, 100);

	nAdcVal = analogRead(DIFFCHN4IN);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[6] = map(nAdcVal, 5, 45, 0, 100);

	nAdcVal = analogRead(DIFFCHN4OUT);
	nAdcVal = map(nAdcVal, 0, 1024, 0, 50);
	currStatus.diffPress[7] = map(nAdcVal, 5, 45, 0, 100);
}


void readFenceVoltage()
{

	float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */
	int16_t results = 0;

	// 1st ADS1115
	results = ads1.readADC_Differential_0_1();
	currStatus.zoneVolt[0] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

	results = ads1.readADC_Differential_2_3();
	currStatus.zoneVolt[1] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

	// 2nd ADS1115
	results = ads2.readADC_Differential_0_1();
	currStatus.zoneVolt[2] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

	results = ads2.readADC_Differential_2_3();
	currStatus.zoneVolt[3] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

	// 3rd ADS1115
	results = ads3.readADC_Differential_0_1();
	currStatus.zoneVolt[4] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

	results = ads3.readADC_Differential_2_3();
	currStatus.zoneVolt[5] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

	// 4th ADS1115
	results = ads4.readADC_Differential_0_1();
	currStatus.zoneVolt[6] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

	results = ads4.readADC_Differential_2_3();
	currStatus.zoneVolt[7] = mapdouble(results * multiplier, 0, VOLTTOP, 0.0, REALTOP);

}


// Display Time 
void displayTime()
{
	DateTime now = rtc.now();
	String timeString = "";

	if (now.hour() < 10)
		timeString += F("0");
	timeString += String(now.hour(), DEC);
	timeString += F(":");
	if (now.minute() < 10)
		timeString += F("0");
	timeString += String(now.minute(), DEC);
	timeString += F(":");

	if (now.second() < 10)
		timeString += F("0");
	timeString += String(now.second(), DEC);
	lcd01.setCursor(0, 0);
	lcd01.print(F("GREENHOUSE1 "));
	lcd01.print(timeString);
	Serial.println(timeString);
}


// 
void RefreshScreen()
{
	// TEMP ROW
	lcd01.setCursor(0, 1);
	String tempStr = "";
	if (isnan(currStatus.temperature))
	{
		lcd01.print(F("TEMP=ERR            "));
	}
	else
	{
		lcd01.print(F("TEMP=    "));
		if (currStatus.temperature >= 0 && currStatus.temperature < 10)
			tempStr += " ";

		tempStr += String(currStatus.temperature, 0);
		lcd01.print(tempStr);
		lcd01.print((char)223);
		lcd01.print("C    ");
	}

	// HUMI ROW
	lcd01.setCursor(0, 2);
	lcd01.print(F("HUMIDITY="));
	String humiStr = "";
	if (currStatus.humidity < 10)
		humiStr += " ";
	if (currStatus.humidity <= 99.9)
	{
		humiStr += String(currStatus.humidity, 0);
		lcd01.print(humiStr);
		lcd01.print("% FAN ");
	}
	else
	{
		lcd01.print("ERR FAN ");
	}


	if (currStatus.bFan)
		lcd01.print("ON ");
	else
		lcd01.print("OFF");

	// Solenoid Status
	lcd01.setCursor(0, 3);
	lcd01.print(F("S0 "));
	if (currStatus.bSol[0])
		lcd01.print(F("O "));
	else
		lcd01.print(F("C "));
	lcd01.print(F("S1 "));
	if (currStatus.bSol[1])
		lcd01.print(F("O "));
	else
		lcd01.print(F("C "));
	lcd01.print(F("S2 "));
	if (currStatus.bSol[2])
		lcd01.print(F("O "));
	else
		lcd01.print(F("C "));
	lcd01.print(F("S3 "));
	if (currStatus.bSol[3])
		lcd01.print(F("O "));
	else
		lcd01.print(F("C "));

	// Zone Display
	lcd02.setCursor(0, 0);
	lcd02.print(F(" ZNE1A="));
	if (currStatus.zoneVolt[0] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[0]);
	lcd02.print(F("V ZNE1B="));
	if (currStatus.zoneVolt[1] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[1]);
	lcd02.print(F("V "));

	lcd02.setCursor(0, 1);
	lcd02.print(F(" ZNE2A="));
	if (currStatus.zoneVolt[2] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[2]);
	lcd02.print(F("V ZNE2B="));
	if (currStatus.zoneVolt[3] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[3]);
	lcd02.print(F("V "));

	lcd02.setCursor(0, 2);
	lcd02.print(F(" ZNE3A="));
	if (currStatus.zoneVolt[4] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[4]);
	lcd02.print(F("V ZNE3B="));
	if (currStatus.zoneVolt[5] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[5]);
	lcd02.print(F("V "));

	lcd02.setCursor(0, 3);
	lcd02.print(F(" ZNE4A="));
	if (currStatus.zoneVolt[6] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[6]);
	lcd02.print(F("V ZNE4B="));
	if (currStatus.zoneVolt[7] < 10)
		lcd02.print(F(" "));
	lcd02.print(currStatus.zoneVolt[7]);
	lcd02.print(F("V "));
}



void initLCD()
{
	lcd01.begin();
	lcd02.begin();
	lcd01.clear();
	lcd02.clear();

	// Logo Screen
	lcd01.setCursor(0, 0);
	lcd01.print(F("                   "));
	lcd01.setCursor(0, 1);
	lcd01.print(F(" GREENHOUSE CONTROL"));
	lcd01.setCursor(0, 2);
	lcd01.print(F("    SYSTEM V1.0    "));
	lcd01.setCursor(0, 3);
	lcd01.print(F("   BY KIERAN 2017  "));

	lcd02.setCursor(0, 0);
	lcd02.print(F("                   "));
	lcd02.setCursor(0, 1);
	lcd02.print(F("    FRESHCARGOT   "));
	lcd02.setCursor(0, 2);
	lcd02.print(F("           "));
	lcd02.setCursor(0, 3);
	lcd02.print(F("           "));
}

void clearLCD()
{
	lcd01.clear();
	lcd02.clear();

}

double mapdouble(double x, double in_min, double in_max, double out_min, double out_max)
{
	if (x <= in_min)
		return out_min;

	if (x >= in_max)
		return out_max;

	if (in_max - in_min == 0.0F)
		return 0;


	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
