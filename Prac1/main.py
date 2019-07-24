#!/usr/bin/python3
"""
Names: Dylan Fouche
Student Number: FCHDYL001
Prac: Prac 1
Date: 22/07/2019
"""

# import Relevant Librares
import RPi.GPIO as GPIO
from time import sleep

#integer to store counter value
count = 0
#bools to store led state
bit_0 = 0
bit_1 = 0
bit_2 = 0

def init_GPIO():
    #config GPIO
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    #config inputs
    GPIO.setup(5, GPIO.IN)
    GPIO.setup(6, GPIO.IN)
    #config outputs
    GPIO.setup(17, GPIO.OUT)
    GPIO.setup(27, GPIO.OUT)
    GPIO.setup(22, GPIO.OUT)
    #config interrupts
    GPIO.add_event_detect(5, GPIO.RISING, callback=increment, bouncetime=500)
    GPIO.add_event_detect(6, GPIO.RISING, callback=decrement, bouncetime=500)
    #tracing
    print("GPIO configured.")

def increment(arg):
    global count
    count += 1

def decrement(arg):
    global count
    count -= 1

def updateLEDs():
    global bit_0
    global bit_1
    global bit_2
    #update led state
    GPIO.output(17,bit_0)
    GPIO.output(27,bit_1)
    GPIO.output(22,bit_2)

def main():
    global count
    global bit_0
    global bit_1
    global bit_2
    #update binary values
    if count & 0b1:
        bit_0 = 1
    else:
        bit_0 = 0
    if count & 0b10:
        bit_1 = 1
    else:
        bit_1 = 0
    if count & 0b100:
        bit_2 = 1
    else:
        bit_2 = 0
    #reflect change on leds
    updateLEDs()

if __name__ == "__main__":
    try:
        #set up GPIOs
        init_GPIO()
        #loop our main function indefinitely
        while True:
            main()
    except KeyboardInterrupt:
        print("Exiting gracefully")
        # Turn off GPIOs
        GPIO.cleanup()
    except e:
        # Turn off GPIOs
        GPIO.cleanup()
        print("Some other error occurred")
        print(e.message)
