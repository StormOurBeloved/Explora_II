#include <SD.h>
#include <time.h>
//error, multiple instances of SD.h

#include <SPI.h>
#include "TMRpcm.h"

#define PIN_SPI_CS 5

#define ERRORPIN 2

float data[8];
//T1, T2, P, Height, Hum, CO2, UV, Albedo

/*
GND - GND
VCC - 3.3V
MISO - 23
MOSI - 19
SCK - 18
CS - PIN_SPI_CS 5??
*/

File myFile;
int Loop = 0;
int TID = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);


  if(!SD.begin(PIN_SPI_CS)){
    Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
    digitalWrite(ERRORPIN, HIGH);
    while(1);//Stuck in loop
  }

  Serial.println(F("SD CARD INITIALIZED."));



  if (!SD.exists("log.txt")) {
    Serial.println(F("arduino.txt doesn't exist. Creating log.txt file..."));
    // create a new file by opening a new file and immediately close it
    myFile = SD.open("log.txt", FILE_WRITE);
    myFile.close();
  }

  if (SD.exists("log.txt")) {
    Serial.println(F("arduino.txt does exist."));
    
  }

  /*
  // recheck if file is created or not
  if (SD.exists("arduino.txt")){
    Serial.println(F("arduino.txt exists on SD Card."));
    myFile = SD.open("arduino.txt", FILE_WRITE);
    myFile.println("Text very cool\n");
    myFile.close();
  }
  else{
    Serial.println(F("arduino.txt doesn't exist on SD Card."));
  }
  */
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //time(Tiden);
  //delay(1000);
  Loop++;  

  if(Serial.available() && Serial.readStringUntil('\n') == "T"){
    
    TID = millis();
    
    myFile = SD.open("log.txt", FILE_WRITE);
    myFile.println("Time: ");
    myFile.println(TID);
    myFile.println("\n");
    myFile.close();

    Serial.print("\nSaved to file - Time: ");
    Serial.print(millis()/1000.f);
  }

}
