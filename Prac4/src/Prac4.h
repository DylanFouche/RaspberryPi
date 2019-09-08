/*
 * Prac4.cpp
 *
 * Written for EEE3096S 2019 by Keegan Crankshaw
 * 
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef PRAC4_H
#define PRAC4_H

//Includes
#include <wiringPi.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <iostream>

//Define buttons
#define PLAY_BUTTON 2
#define STOP_BUTTON 3
#define BUFFER_SIZE 1000

//SPI Settings
#define SPI_CHAN 0
#define SPI_SPEED 819200

//Debounce time
#define DEBOUNCE_TIME 200

//Filename
#define FILENAME "src/sound_16k_8bit.raw"

//Function definitions
void play_audio(void);
void stop_audio(void);
int setup_gpio(void);
int main(void);

//global variables
long last_play_interrupt = 0;
long last_stop_interrupt = 0;

#endif
