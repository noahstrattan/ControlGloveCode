/*
 Noah Strattan
 ED2 Glove code
 Reads data from wii sensor and flex sensors and sends over wireless
 Last revised: 03-27-2012
*/

// Headers
#include <Wire.h>  // Used for nunchuck
#include <SoftwareSerial.h>  // Used for virtual serial
#include "nunchuck_funcs.h"  // Wii nunchuck library

#define smoothing 3

// Softwareserial
SoftwareSerial xBeeSerial(2,3);

// Variables
int finger = 0, thumb = 0;  // Flex values
byte wiiX, wiiY, grip;
byte serialData[4];  // Servo values

// Arrays for smoothing
byte wiiYArray[smoothing]={0,0,0};
int indexY=0;
byte wiiXArray[smoothing];
int indexX=0;

// Setup stuff
void setup()
{
  // Open serial ports
  Serial.begin(9600);  // Hardware serial
  xBeeSerial.begin(19200);  // Software serial
  
  // Setup nunchuck
  nunchuck_init();
}
void loop()
{
  // Read all values from accelerometer
  nunchuck_get_data();
  
  // Calculate servo positions (normalize on scale from 0-180)
  // EX: wiiX = ((accelerometerX - XminVal) * (XmaxVal - XminVal)/180)
  wiiX = (nunchuck_accelx() - 72) * 1.6;   // nunchuck_accelx() ranges from 72 - 182
  wiiY = 180 - ((nunchuck_accely() - 69) * 1.55);  // nunchuck_accely() ranges from 69 - 183
  
  finger = analogRead(A0);  // Ranges from 525 - 300
  thumb = analogRead(A1);  // Ranges from 550 - 400
  thumb = (thumb - 220) * 1.5;
  
  grip = (thumb + finger)/2;  // average. range 75 - 250
  grip = 180 - ((grip - 70) * 1.05);  // normalize to 0-180
  
  Serial.print(wiiY);
  Serial.print("\t");
  // Smooth using running avg (testing on wiiY)
  smoothParameter(&wiiY, wiiYArray, &indexY);
  smoothParameter(&wiiX, wiiXArray, &indexX);
  
  Serial.println(wiiY);
  
  /*
  totalY = totalY - wiiYArray[indexY];
  wiiYArray[indexY] = wiiY;
  totalY = totalY + wiiYArray[indexY];
  indexY++;
  if(indexY >= smoothing){
    indexY = 0;
  }
  avgY = totalY / smoothing;
  */
  
  
  // Set serialData array
  serialData[0] = 225;
  serialData[1] = wiiY;
  serialData[2] = wiiX;
  serialData[3] = grip;
  // Serial write data
  xBeeSerial.write(serialData,4); 
  delay(100);
}

void smoothParameter(byte* value, byte dataArray[smoothing],int* index){
  int temp = 0;
  dataArray[*index] = *value;
  for(int n=0; n<smoothing; n++){
    temp = temp + dataArray[n];
  }
  *value = temp / smoothing;
  *index = *index + 1;
  if(*index >= smoothing){
    *index = 0;
  }
}

