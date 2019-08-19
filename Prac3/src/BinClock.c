/*
 * BinClock.c
 * Jarrod Olivier
 * Modified for EEE3095S/3096S by Keegan Crankshaw
 * August 2019
 *
 * FCHDYL001
 * 19/08/2019
*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <softPwm.h>

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int hours, mins, secs;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance
int HH,MM,SS;

void initGPIO(void){
	/*
	 * Sets GPIO using wiringPi pins. see pinout.xyz for specific wiringPi pins
	 */

	printf("Setting up\n");

	//setup wiringPi in default mode
	wiringPiSetup();

	//Set up the RTC
	RTC = wiringPiI2CSetup(RTCAddr);

	//Set up the minutes and hours LEDS
	for(int i; i < sizeof(LEDS)/sizeof(LEDS[0]); i++){
	    pinMode(LEDS[i], OUTPUT);
	}

	//Set Up the Seconds LED for PWM
	softPwmCreate(SECS,0,59);

	printf("LEDS done\n");

	//Set up the Buttons
	for(int j; j < sizeof(BTNS)/sizeof(BTNS[0]); j++){
		pinMode(BTNS[j], INPUT);
		pullUpDnControl(BTNS[j], PUD_UP);
	}

	//Attach interrupts to Buttons
	wiringPiISR(5, INT_EDGE_RISING, &minInc);
	wiringPiISR(30, INT_EDGE_RISING, &hourInc);

	printf("BTNS done\n");
	printf("Setup done\n");
}

int main(void){
	//call gpio init function
	initGpio();

	//Set random starting time on RTC (3:04PM)
	wiringPiI2CWriteReg8(RTC, HOUR, 0x13+TIMEZONE);
	wiringPiI2CWriteReg8(RTC, MIN, 0x4);
	wiringPiI2CWriteReg8(RTC, SEC, 0x00);

	//Start the RTC counting
	wiringPiI2CWriteReg8(RTC,SEC,0b10000000);

	//Loop until we quit
	for (;;){
		//Fetch the time from the RTC
		HH = wiringPiI2CReadReg8(RTC,HOUR);
		MM = wiringPiI2CReadReg8(RTC,MIN);
		SS = wiringPiI2CReadReg8(RTC,SEC);

		//Grab the important bits and convert to decimal
		hours = hexCompensation(HH & 0b00111111);
		mins = hexCompensation(MM & 0b01111111);
		secs = hexCompensation(SS & 0b01111111);

		//Display updated time on LEDs
		lightHours();
		lightMins();
		secPWM();

		// Print out the time we have stored on our RTC
		printf("The current time is: %d:%d:%d\n", hours, mins, secs);

		//using a delay to make our program "less CPU hungry"
		delay(1000); //milliseconds
	}
	return 0;
}

int hFormat(int hours){
	/*
	 * change hours format to 12h
	 */

	if (hours >= 24){
		hours = 0;
	}
	else if (hours > 12){
		hours -= 12;
	}
	return (int)hours;
}

void lightHours(){
	/*
	 * display hours binary value on LED array
	 */

	int hours_12 = hFormat(hours);
	//get each bit from hour value
	int h0 = hours_12 & 0b1;
	int h1 = hours_12 & 0b10;
	int h2 = hours_12 & 0b100;
	int h3 = hours_12 & 0b1000;
	//write bits to led outputs
	digitalWrite(LEDS[0],h3);
	digitalWrite(LEDS[1],h2);
	digitalWrite(LEDS[2],h1);
	digitalWrite(LEDS[3],h0);
}

void lightMins(int units){
	/*
	 * display minutes binary value on LED array
	 */

	//get each bit from minute value
	int m0 = mins & 0b1;
	int m1 = mins & 0b10;
	int m2 = mins & 0b100;
	int m3 = mins & 0b1000;
	int m4 = mins & 0b10000;
	int m5 = mins & 0b100000;
	//write bits to led outputs
	digitalWrite(LEDS[4],m5);
	digitalWrite(LEDS[5],m4);
	digitalWrite(LEDS[6],m3);
	digitalWrite(LEDS[7],m2);
	digitalWrite(LEDS[8],m1);
	digitalWrite(LEDS[9],m0);
}

void secPWM(){
	/*
	 * Update pwm value on seconds led
	 */

	softPwmWrite(SECS,secs);
}

int hexCompensation(int units){
	/*
         * Convert HEX or BCD value to DEC where 0x45 == 0d45
	 */

	int unitsU = units%0x10;
	if (units >= 0x50){
		units = 50 + unitsU;
	}
	else if (units >= 0x40){
		units = 40 + unitsU;
	}
	else if (units >= 0x30){
		units = 30 + unitsU;
	}
	else if (units >= 0x20){
		units = 20 + unitsU;
	}
	else if (units >= 0x10){
		units = 10 + unitsU;
	}
	return units;
}

int decCompensation(int units){
	/*
	 * Convert DEC to HEX value for writing through I2C
	 */

	int unitsU = units%10;
	if (units >= 50){
		units = 0x50 + unitsU;
	}
	else if (units >= 40){
		units = 0x40 + unitsU;
	}
	else if (units >= 30){
		units = 0x30 + unitsU;
	}
	else if (units >= 20){
		units = 0x20 + unitsU;
	}
	else if (units >= 10){
		units = 0x10 + unitsU;
	}
	return units;
}

void hourInc(void){
	/*
	 * Interrupt handler to increment hour value and write back to RTC
	 */

	//software debounce
	long interruptTime = millis();
	if (interruptTime - lastInterruptTime>300){
		//increment hours
		hours++;
		if(hours>=24){hours=0;}
		//write new val to RTC
		wiringPiI2CWriteReg8(RTC, HOUR, decCompensation(hours));
		//display interrupt message
		printf("Interrupt 1 triggered, hours => %d\n", hours);
	}
	lastInterruptTime = interruptTime;
}

void minInc(void){
	/*
	 * Interrupt handler to increment minute value and write back to RTC
	 */

	//software debounce
	long interruptTime = millis();
	if (interruptTime - lastInterruptTime>300){
		//increment minutes
		mins++;
		if(mins>=60){mins=0;}
		//write new val to RTC
		wiringPiI2CWriteReg8(RTC, MIN, decCompensation(mins));
		//display interrupt message
		printf("Interrupt 2 triggered, minutes => %d\n", mins);
	}
	lastInterruptTime = interruptTime;
}

//This interrupt will fetch current time from another script and write it to the clock registers
//This functions will toggle a flag that is checked in main
void toggleTime(void){
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		HH = getHours();
		MM = getMins();
		SS = getSecs();

		HH = hFormat(HH);
		HH = decCompensation(HH);
		wiringPiI2CWriteReg8(RTC, HOUR, HH);

		MM = decCompensation(MM);
		wiringPiI2CWriteReg8(RTC, MIN, MM);

		SS = decCompensation(SS);
		wiringPiI2CWriteReg8(RTC, SEC, 0b10000000+SS);

	}
	lastInterruptTime = interruptTime;
}
