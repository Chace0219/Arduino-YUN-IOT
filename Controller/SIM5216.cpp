#include "SIM5216.h"
#include <Arduino.h>

int8_t answer;
int onModulePin = 8;
char aux_str[30];

const char phone_number[] = "17191686920";

void gsmInit()
{
	pinMode(onModulePin, OUTPUT);
	Serial2.begin(115200);
	delay(2000);
	Serial.println("GSM Init");
}

void sendSMS(String smsMsg)
{
	delay(500);
	power_on();
	delay(3000);
	Serial.println("Connecting to the network...");
	while ((sendATcommand("AT+CREG?", "+CREG: 0,1", 500) ||
		sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0);
	Serial.print("Setting SMS mode...");
	sendATcommand("AT+CMGF=1", "OK", 1000);    // sets the SMS mode to text
	Serial.println("Sending SMS");
	sprintf(aux_str, "AT+CMGS=\"%s\"", phone_number);
	answer = sendATcommand(aux_str, ">", 4000);    // send the SMS 
	{
		Serial2.println(smsMsg.c_str());
		Serial2.write(0x1A);
		answer = sendATcommand("", "OK", 40000);
		if (answer == 1)
		{
			Serial.print("Sent ");
		}
		else
		{
			Serial.print("error ");
		}
	}
}

void power_on() {

	uint8_t answer = 0;

	// checks if the module is started
	answer = sendATcommand("AT", "OK", 4000);
	if (answer == 0)
	{
		// power on pulse
		digitalWrite(onModulePin, HIGH);
		delay(5000);
		digitalWrite(onModulePin, LOW);

		// waits for an answer from the module
		while (answer == 0) {     // Send AT every two seconds and wait for the answer
			answer = sendATcommand("AT", "OK", 4000);
		}
	}
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {

	uint8_t x = 0, answer = 0;
	char response[100];
	unsigned long previous;

	memset(response, '\0', 100);    // Initialice the string

	delay(100);

	while (Serial2.available() > 0) Serial2.read();    // Clean the input buffer

	Serial2.println(ATcommand);    // Send the AT command 
								   /*
								   while( Serial2.available() > 0) Serial2.read();    // Clean the input buffer

								   Serial2.println(ATcommand);    // Send the AT command
								   */
	x = 0;
	previous = millis();

	// this loop waits for the answer
	do {
		// if there are data in the UART input buffer, reads it and checks for the answer
		if (Serial2.available() != 0) {
			response[x] = Serial2.read();
			x++;
			// check if the desired answer is in the response of the module
			if (strstr(response, expected_answer) != NULL)
			{
				answer = 1;
			}
		}
		// Waits for the answer with time out
	} while ((answer == 0) && ((millis() - previous) < timeout));

	return answer;
}
