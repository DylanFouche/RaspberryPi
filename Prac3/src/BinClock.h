#ifndef BINCLOCK_H
#define BINCLOCK_H

// Function definitions
int hFormat(int hours);
void lightHours();
void lightMins();
int hexCompensation(int units);
int decCompensation(int units);
void initGPIO(void);
void secPWM();
void hourInc(void);
void minInc(void);
void toggleTime(void);

// define constants
const char RTCAddr = 0x6f;
const char SEC = 0x00; // see register table in datasheet
const char MIN = 0x01;
const char HOUR = 0x02;
const char TIMEZONE = 2; // +02H00 (RSA)

// define pins
const int LEDS[] = {0,2,3,25,7,22,21,27,4,6}; //H0-H3, M0-M5
const int SECS = 1;
const int BTNS[] = {5,30}; // B0, B1


#endif
