/*
 Florida Atlantic University
 Teleoperated Robotic Arm
 Noah Strattan
 Last modified: 04-30-2012
 
 Control Glove code
 Reads data from wii sensor and flex sensors and sends over wireless
 
 Setup:
 Arduino Nano
 xBee module using pins 2,3 for software serial
 Wii Nunchuck accelerometer
 Flex sensors on pins A0,A1
 Feedback motor on pin 5
*/

// Libraries
#include <Wire.h>  // Used for nunchuck
#include <SoftwareSerial.h>  // Used for virtual serial
#include "nunchuck_funcs.h"  // Wii nunchuck library

#define smoothing 3  // Set smoothing coeffecient

SoftwareSerial xBeeSerial(2,3);  // Software serial for xBee comm.

// Variables
int finger = 0, thumb = 0;  // Flex values
byte wiiX, wiiY, grip;
byte serialData[5];  // Servo values

// Arrays for smoothing
byte wiiYArray[smoothing];
int indexY=0;
byte wiiXArray[smoothing];
int indexX=0;

int time=0,oldtime=0;

// Setup stuff
void setup()
{
  pinMode(5, OUTPUT);
  // Open serial ports
  Serial.begin(9600);  // Hardware serial
  xBeeSerial.begin(9600);  // Software serial
  
  nunchuck_init();  // Setup nunchuck
}
void loop()
{
  nunchuck_get_data();  // Read all values from accelerometer
  
  // Read sensor values
  wiiX = nunchuck_accelx();  // nunchuck_accelx() ranges from 72 - 182
  wiiY = nunchuck_accely();  // nunchuck_accely() ranges from 69 - 183
  finger = analogRead(A0);   // Ranges from 520 - 300
  thumb = analogRead(A1);    // Ranges from 540 - 440
  
  // Smooth data
  smoothParameter(&wiiX, wiiXArray, &indexX);  // Smooth X
  smoothParameter(&wiiY, wiiYArray, &indexY);  // Smooth Y
  
  // Calculate servo positions (normalize on scale from 0-180)
  // EX: wiiX = ((accelerometerX - XminVal) * (XmaxVal - XminVal)/180)
  wiiX = (nunchuck_accelx() - 70) * 1.6;   
  wiiY = 180 - ((nunchuck_accely() - 70) * 1.4);  
  
  finger = (finger - 315) * 0.85;
  thumb = (thumb - 430) * 1.75;
  grip = 30 + ((thumb + finger)/2);
  
  // Set serialData array
  serialData[0] = 225;  // 225 is a start-of-data flag used by the computer
  serialData[1] = 47;   // Second flag for better error checking
  serialData[2] = wiiY;
  serialData[3] = wiiX;
  serialData[4] = grip;
  xBeeSerial.write(serialData,5);  // Serial write data
  if(xBeeSerial.read() == 127){
    if(xBeeSerial.read() >= 200)
    {
      analogWrite(5, 200);
    }
    else
    {
     analogWrite(5, 0); 
    }
  }
  delay(50);  // Delay is used to not oversample the accelerometer (Fs=10Hz)
}

// Smooths data using a running average
void smoothParameter(byte* value, byte dataArray[smoothing],int* index){
  int temp = 0;
  dataArray[*index] = *value;
  
  // Sum all values
  for(int n=0; n<smoothing; n++){
    temp = temp + dataArray[n];
  }
  *value = temp / smoothing;  // Find avg
  
  *index = *index + 1;
  if(*index >= smoothing){
    *index = 0;
  }
}

