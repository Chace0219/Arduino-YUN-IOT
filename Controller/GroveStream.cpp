#include "GroveStream.h"
#include "measure.h"
#include "QueueList.h"

// GroveStreams Settings, You have to change mac id and api key as fit to you device and grove stream component
char mac[] = "A8:40:41:15:B8:CC"; // Change this!!! Must be unique on local network.
char gsApiKey[] = "b8194a66-ebad-31af-b786-09447ece0588";   //Change This!!!
															// Component settings
char gsComponentName[] = "Greenhouse";
char gsComponentTemplateId[] = "temp";
// create a queue of strings messages.
QueueList <String> queue;

void zoneVoltageStream()
{
	String query = "";
	query += "&Z1A=";
	query += String(currStatus.zoneVolt[0], DEC);
	query += "&Z1B=";
	query += String(currStatus.zoneVolt[1], DEC);
	query += "&Z2A=";
	query += String(currStatus.zoneVolt[2], DEC);
	query += "&Z2B=";
	query += String(currStatus.zoneVolt[3], DEC);
	updateGroveStreams(query);

	query = "";
	query += "&Z3A=";
	query += String(currStatus.zoneVolt[4], DEC);
	query += "&Z3B=";
	query += String(currStatus.zoneVolt[5], DEC);
	query += "&Z4A=";
	query += String(currStatus.zoneVolt[6], DEC);
	query += "&Z4B=";
	query += String(currStatus.zoneVolt[7], DEC);
	updateGroveStreams(query);
}

void zoneAlarmStream(uint8_t idxZone, uint8_t nValue)
{
	//Z1_ALARM
	String id = "Z" + String(idxZone + 1);
	id += "_ALARM";

	String query = "";
	query += "&";
	query += id;
	query += "=";
	query += String(nValue);
	updateGroveStreams(query);
}

void solenoidStream()
{
	String query = "";
	query += "&S0=";
	query += String(currStatus.bSol[0], DEC);
	query += "&S1=";
	query += String(currStatus.bSol[1], DEC);
	query += "&S2=";
	query += String(currStatus.bSol[2], DEC);
	query += "&S3=";
	query += String(currStatus.bSol[3], DEC);
	updateGroveStreams(query);
}

void temperStream()
{
	String query = "";
	query += "&TEMP=";
	query += String(currStatus.temperature, 1);
	query += "&HUMI=";
	query += String(currStatus.humidity, 0);

	query += "&F1IN=";
	query += String(currStatus.diffPress[0], DEC);
	query += "&F1OUT=";
	query += String(currStatus.diffPress[1], DEC);

	updateGroveStreams(query);
	query = "";
	query += "&F2IN=";
	query += String(currStatus.diffPress[2], DEC);
	query += "&F2OUT=";
	query += String(currStatus.diffPress[3], DEC);

	query += "&F3IN=";
	query += String(currStatus.diffPress[4], DEC);
	query += "&F3OUT=";
	query += String(currStatus.diffPress[5], DEC);

	updateGroveStreams(query);
	query = "";

	query += "&F4IN=";
	query += String(currStatus.diffPress[6], DEC);
	query += "&F4OUT=";
	query += String(currStatus.diffPress[7], DEC);

	updateGroveStreams(query);
}

/* */
void procStreamList()
{
	if (!queue.isEmpty())
	{
		updateGroveStreams(queue.pop());
	}
}

/* */
void addStreamList(String QueryString)
{
	queue.push(QueryString);
}

/* */
void updateGroveStreams(String queryString)
{

	Serial.println("Uploading stream data ");
	Serial.println(queryString);
	//You may need to increase the size of urlBuf if any other char array sizes have increased
	char urlBuf[180];
	sprintf(urlBuf, "http://grovestreams.com/api/feed?compTmplId=%s&compId=%s&compName=%s&api_key=%s%s",
		gsComponentTemplateId, mac, gsComponentName, gsApiKey, queryString.c_str());

	char xHeadBuf[40];
	sprintf(xHeadBuf, "X-Forwarded-For:%s\0", mac);

	Process process;
	process.begin("curl");
	process.addParameter("-d");
	process.addParameter("");
	process.addParameter("-k");
	process.addParameter("-X");
	process.addParameter("PUT");

	//Headers
	process.addParameter("-H");
	process.addParameter(xHeadBuf);
	process.addParameter("-H");
	process.addParameter("Connection:close");
	process.addParameter("-H");
	process.addParameter("Content-Type:application/json");

	//URL
	process.addParameter(urlBuf);

	//Make the request
	unsigned int result = process.run();

	//Display whatever is returned (usually an error message if one occurred)

	if (process.available() > 0)
		addStreamList(queryString);
	while (process.available() > 0) 
	{
		char c = process.read();
		Serial.print(c);
	}
	Serial.flush();


	Serial.print("Feed PUT Result: ");
	Serial.println(result);
}

void fanStream()
{
	String query = "";
	query += "&FAN=";
	query += String(currStatus.bFan, DEC);
	updateGroveStreams(query);
}
