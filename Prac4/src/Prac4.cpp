
/*
 * Prac4.cpp
 *
 * Original written by Stefan Schröder and Dillion Heald
 *
 * Adapted for EEE3096S 2019 by Keegan Crankshaw
 *
 * Implemented by Dylan Fouche
 * UCT CS3
 * FCHDYL001
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

#include "Prac4.h"

using namespace std;

bool playing = true; // should be set false when paused
bool stopped = false; // If set to true, program should close
unsigned char buffer[2][BUFFER_SIZE][2];
int buffer_location = 0;
bool bufferReading = 0; //using this to switch between column 0 and 1 - the first column
bool threadReady = false; //using this to finish writing the first column at the start of the song, before the column is played

/*
 *Interrupt handlers
 */
void play_pause_isr(void){
    long interrupt_time = millis();
    //software debounce
    if(interrupt_time - last_play_interrupt > DEBOUNCE_TIME){
        playing = !playing;
        if(playing){
	    printf("Playing\n");
        }
        else{
            printf("Paused\n");
        }
    }
    last_play_interrupt = interrupt_time;
}
void stop_isr(void){
    long interrupt_time = millis();
    //software debounce
    if(interrupt_time - last_stop_interrupt > DEBOUNCE_TIME){
        stopped = !stopped;
        if(stopped){
	    printf("Stopped\n");
	    exit(0);
        }
        else{
            printf("Started\n");
        }
    }
    last_stop_interrupt = interrupt_time;
}

/*
 * GPIO Setup Function.
 */
int setup_gpio(void){
    //Set up wiring Pi
    wiringPiSetup();
    //setting up the buttons
    pinMode(PLAY_BUTTON, INPUT);
    pullUpDnControl(PLAY_BUTTON, PUD_UP);
    pinMode(STOP_BUTTON, INPUT);
    pullUpDnControl(STOP_BUTTON, PUD_UP);
    //registering interrupt handlers for buttons
    wiringPiISR(PLAY_BUTTON, INT_EDGE_RISING, &play_pause_isr);
    wiringPiISR(STOP_BUTTON, INT_EDGE_RISING, &stop_isr);
    //setting up the SPI interface
    wiringPiSPISetup (SPI_CHAN, SPI_SPEED);
    return 0;
}

/*
 * Thread that handles writing to SPI
 */
void *playThread(void *threadargs){
    // If the thread isn't ready, don't do anything
    while(!threadReady)
        continue;
    //only be playing if the stopped flag is false
    while(!stopped){
        if(playing){
        //Write the buffer out to SPI
	wiringPiSPIDataRW (SPI_CHAN, (unsigned char*)buffer[bufferReading][buffer_location], 2);
        //Do some maths to check if you need to toggle buffers
        buffer_location++;
        if(buffer_location >= BUFFER_SIZE) {
            buffer_location = 0;
            bufferReading = !bufferReading; // switches column one it finishes one column
        }
	}
    }
    //terminate thread
    pthread_exit(NULL);
}

/*
 * main function
 */
int main(){
    // Call the setup GPIO function
	if(setup_gpio()==-1){
        return 0;
    }
    // Initialize our pthread
    pthread_attr_t tattr;
    pthread_t thread_id;
    int newprio = 99;
    sched_param param;
    pthread_attr_init (&tattr);
    pthread_attr_getschedparam (&tattr, &param); /* safe to get existing scheduling param */
    param.sched_priority = newprio; /* set the priority; others are unchanged */
    pthread_attr_setschedparam (&tattr, &param); /* setting the new scheduling param */
    pthread_create(&thread_id, &tattr, playThread, (void *)1); /* with new priority specified */

    // Open the file
    char ch;
    FILE *filePointer;
    printf("%s\n", FILENAME);
    filePointer = fopen(FILENAME, "r"); // read mode

    if (filePointer == NULL) {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    int counter = 0;
    int bufferWriting = 0;

    // Reading from file, character by character
	while((ch = fgetc(filePointer)) != EOF){
            while(threadReady && bufferWriting==bufferReading && counter==0){
            //waits in here after it has written to a side, and the thread is still reading from the other side
            continue;
        }
        if(playing){
	    //buffer[bufferWriting][counter][0] gets control bits and top 4 bits of audio
            buffer[bufferWriting][counter][0] = 0b01010000 | (ch>>4);
	    //buffer[bufferWriting][counter][1] gets bottom 4 bits of audio and padding zeroes
            buffer[bufferWriting][counter][1] = ch<<4;
            if(++counter >= BUFFER_SIZE+1){
                if(!threadReady){
                    threadReady = true;
                }
                counter = 0;
                bufferWriting = (bufferWriting+1)%2;
            }
        }

    }

    // Close the file
    fclose(filePointer);
    printf("Complete reading");

    //Join and exit the playthread
    pthread_join(thread_id, NULL);
    pthread_exit(NULL);

    return 0;
}

