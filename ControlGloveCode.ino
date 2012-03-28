/*
 Noah Strattan
 ED2 Glove code
 Reads data from wii sensor and flex sensors and sends over wireless
 Last revised: 03-28-2012
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
byte serialData[4];  // Servo values

// Arrays for smoothing
byte wiiYArray[smoothing];
int indexY=0;
byte wiiXArray[smoothing];
int indexX=0;

int time=0,oldtime=0;

// Setup stuff
void setup()
{
  // Open serial ports
  Serial.begin(9600);  // Hardware serial
  xBeeSerial.begin(19200);  // Software serial
  
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
  wiiX = (nunchuck_accelx() - 72) * 1.6;   
  wiiY = 180 - ((nunchuck_accely() - 69) * 1.55);  
  
  finger = (finger - 315) * 0.85;
  thumb = (thumb - 430) * 1.75;
  grip = (thumb + finger)/2;
  
  // Set serialData array
  serialData[0] = 225;  // 225 is a start-of-data flag used by the computer
  serialData[1] = wiiY;
  serialData[2] = wiiX;
  serialData[3] = grip;
  
  xBeeSerial.write(serialData,4);  // Serial write data
  delay(97);  // Delay is used to not oversample the accelerometer (Fs=10Hz)
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

