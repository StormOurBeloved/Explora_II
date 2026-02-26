//this
/*¨
#include <SoftwareSerial.h>

#include <TinyGPS.h>

float latitude, longitude;

SoftwareSerial gpSerial(10,11); //Rx, Tx

TinyGPS gps;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Testing stuffs.");

  gpSerial.begin(9600);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  while(gpSerial.available()){
    Serial.println(gpSerial.read());
    //if(gps.encode(gpSerial.read())){
    
      gps.f_get_position(&latitude, &longitude);

      Serial.print("Lat: ");
      Serial.println(latitude);
      Serial.print("Long: ");
      Serial.println(longitude);
    //}
    delay(1000);
    //this*/

    //other
    /*
    if(gps.encode(gpSerial.read())){

      gps.f_get_position(&latitude, &longitude);

      Serial.print(latitude);
      Serial.print(",");
      Serial.println(longitude);
    }
    */

  //this}

//this}

#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(10, 11);
unsigned char buffer[64];                   // buffer array for data receive over serial port
int count=0;                                // counter for buffer array
void setup()
{
    SoftSerial.begin(9600);                 // the SoftSerial baud rate
    Serial.begin(9600);                     // the Serial port of Arduino baud rate.
    //Serial.println("Starting up...");
}

void loop()
{
    //delay(1000);

    if (SoftSerial.available())                     // if date is coming from software serial port ==> data is coming from SoftSerial shield
    {
        while(SoftSerial.available())               // reading data into char array
        {
            buffer[count++]=SoftSerial.read();      // writing data into array
            if(count == 64)break;
        }
        Serial.write(buffer,count);                 // if no data transmission ends, write buffer to hardware serial port
        clearBufferArray();                         // call clearBufferArray function to clear the stored data from the array
        count = 0;                                  // set counter of while loop to zero 
    }
    if (Serial.available())                 // if data is available on hardware serial port ==> data is coming from PC or notebook
    SoftSerial.write(Serial.read());        // write it to the SoftSerial shield
}


void clearBufferArray()                     // function to clear buffer array
{
    for (int i=0; i<count;i++)
    {
        buffer[i]=NULL;
    }                      // clear all index of array with command NULL
}





