#ifndef SIM5216_H
#define SIM5216_H

#include <Arduino.h>

void gsmInit();
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout);
void power_on();
void sendSMS(String smsMsg);

#endif 
