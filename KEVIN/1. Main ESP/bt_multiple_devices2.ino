//SD headers
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//BT CO2 & MPL3115A2 module & almost all

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
int report = 0x08;
int TID = 0;

bool error = false;
bool BTenabled = true;
bool CAM1 = false;
bool CAM2 = false;

//micro SD card
#include <time.h>

#include <SPI.h>
#define PIN_SPI_CS 5

//ORANGE = SDA
//BLÅ = SCL


#define ERRORPIN 2
#define BTPIN 4

//SD-Card module
File file;
/*
GND - GND
VCC - 3.3V
MISO - 23
MOSI - 19
SCK - 18
CS - PIN_SPI_CS 2?? (ESP32)
*/

//current module (s)
#include <Adafruit_MPL3115A2.h>
/*
Vin
GND
SCL
SDA
*/
#include <SCD30.h>
/*
GND
VCC
SDA
SCL1
*/
//#include "BH1750.h" // _ Cristopher Laws (Arduino IDE)
#include <Wire.h>
#include "BH1750_WE.h" //By french but on libmanage (Wolfgang)

#define BH1750_ADDRESS_1 0x5C  //albedo_indirect
#define BH1750_ADDRESS_2 0x23  //albedo_direct

BH1750_WE myBH1750_1(BH1750_ADDRESS_1); 
BH1750_WE myBH1750_2(BH1750_ADDRESS_2); 
/*
VCC - 3.3V
GND
SCL
SDA
ADDR - nah
*/

Adafruit_MPL3115A2 baro;

//albedo old
//BH1750 Albedo;

//Outside temp
#include "OneWire.h"
#include "DallasTemperature.h"
// Define to which pin of the Arduino the 1-Wire bus is connected:

#define ONE_WIRE_BUS 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//------------------------------------------\\

void writeFile(fs::FS & fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if(file.print(message)){
        Serial.println("File written.");
    } else {
        Serial.println("Write failed.");
    }
  file.close();
}

void writeFileData(fs::FS & fs, const char * path, float T, float P, float A, float T2, float H, float C, float L, float T3){
  File file = fs.open(path, FILE_APPEND);
  TID = millis();
  if(!file.printf("%d, %f, %f, %f, %f, %f, %f, %f, %f\n", TID, T,P,A,T2,H,C,L,T3)){
  //if(!file.print(TID)){
    Serial.println("Failed to write to file.");
  }
  file.close();
}

bool setupFile(fs::FS & fs, const char * path){
  File file = fs.open(path, FILE_WRITE);
    if(!file){
      Serial.println("Failed to open file for writing");
      return false;
    }
    Serial.println("Found file on SD.");
    file.close();
    return true;
}

void showTransmit(bool DoAnyways){
  if(SerialBT.connected() || DoAnyways){
    digitalWrite(BTPIN, HIGH);
    delay(100);
    digitalWrite(BTPIN, LOW);
  }
}

void setup() {
  pinMode(BTPIN, OUTPUT);
  pinMode(ERRORPIN, OUTPUT);

  Serial.begin(115200);
  delay(2000);
  SerialBT.begin("ESP32test-2"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  //wait for BT-connection (up to 1 min)
  Serial.println("Awaiting Bluetooth connection...");
  while(!SerialBT.connected() && millis() < 300000){
    showTransmit(true);
    delay(250);
  }
  delay(500);
  if(SerialBT.connected()){
    Serial.println("Connected to external device.");
    showTransmit(false);
  }
  else{
    Serial.println("Could not secure connection within timeframe.");
  }
  Serial.println("Running system diagnostics...");

  //Inititate SD-card module
  if(!SD.begin()){
      Serial.println("Card Mount Failed");
      return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  if(setupFile(SD, "/log.txt")){
    Serial.println("Succesfully setup log file.");
  }
  else{
    Serial.println("Failed to setup log file.");
    error = true;
  }

  writeFile(SD,"/log.txt", "Time : Temp :  Press : Alt : Temp2 : Hum : CO2 : Alb : Temp3\n");


  //initate sensors
  if (!baro.begin()) {
    Serial.println("ERROR: Failed to setup Barometric sensor.");
    error = true;
  }

  // use to set sea level pressure for current location
  // this is needed for accurate altitude measurement
  // STD SLP = 1013.26 hPa
  baro.setSeaPressure(1013.26);
  //initate SCD30 sensor
  scd30.initialize();
  if(!scd30.isAvailable()){
    Serial.println("ERROR: Failed to setup CO2 sensor.");
    error = true;
  }
  //initiate BH Sensor
  Wire.begin();
  if(!myBH1750_1.init()){
    Serial.println("ERROR: Failed to setup Albedo sensor 1.");
    error = true;
  }
  if(!myBH1750_2.init()){
    Serial.println("ERROR: Failed to setup Albedo sensor 2.");
    error = true;
  }
  /*albedo old
  if(!Albedo.begin()){
    Serial.println("ERROR: Failed to setup Albedo sensor.");
    error = true;
  }
  */
  //Outside temp
  //sensors.begin();

  showTransmit(false);
  if(error){
    Serial.println("1 or more errors when setting up sensors.");
    SerialBT.print("1 or more errors when setting up sensors.\n");
    digitalWrite(ERRORPIN, HIGH);
    while(1){} //This here kind of dumb, bättre att köra med de sensorer som fungerar än att ge upp
  }
  Serial.println("All sensors setup succesfully."); 
  SerialBT.print("All sensors setup succesfully.\n");
  

  //End of setup
}

void loop() {

  delay(250);

  //update sensor values
  TID = millis();



  //MPL
  float t = baro.getTemperature();
  float p = baro.getPressure();
  float a = baro.getAltitude();

  //dht - Replace
  //float t2 = dht.readTemperature();
  //float h = dht.readHumidity();

  //SCD
  float cht[3] = {0};
  scd30.getCarbonDioxideConcentration(cht);


  //BH
  //float l = Albedo.readLightLevel(); albedo old
  float Light_direct = myBH1750_1.getLux();
  float Light_indirect = myBH1750_2.getLux();
  float albedo = (Light_direct/Light_indirect);

  //Outside temp
  sensors.requestTemperatures();
  float t3 = sensors.getTempCByIndex(0);

  //replace 0 & 0 with t2 & hum
  //writeFileData(SD, "/log.txt", t,p,a,0,0,c,l,t3);
  //writeFileData(SD, "/log.txt", t,p,a,cht[2],cht[1],cht[0],l,t3); old albedo
  
  writeFileData(SD, "/log.txt", t,p,a,cht[2],cht[1],cht[0],albedo,t3);

  Serial.print("\nSaved to file - Time: ");
  Serial.print(millis()/1000.f);
  //End of Save to SD-card

  delay(150);
  if(BTenabled){
      if(SerialBT.available()){
        String recieved = SerialBT.readStringUntil('\n');
        showTransmit(false);
        if(recieved == "BTStop"){
          SerialBT.print("Terminating Bluetooth...");
          SerialBT.disconnect();
          delay(200);
          SerialBT.end();
          BTenabled = false;
          Serial.println("\nDisabled Bluetooth.");
        }
      }

    if(!report--){
      //Report via Bluetooth (once every 16 sycles (seconds))
      report = 0x08;
      showTransmit(false);

      SerialBT.print("Temp: ");
      SerialBT.print(String(t));
      SerialBT.print(",  Pres: ");
      SerialBT.print(String(p));
      SerialBT.print(",  Alt: ");
      SerialBT.print(String(a));
      SerialBT.print(",  temp: ");    //test
      SerialBT.print(String(cht[2]));
      SerialBT.print(",  Hum: ");     //test
      SerialBT.print(String(cht[1]));
      SerialBT.print(",  CO2: ");
      SerialBT.print(String(cht[0]));
      SerialBT.print(",  Alb: ");
      //SerialBT.print(String(l)); old albedo
      SerialBT.print(String(albedo));
      SerialBT.print(",  OTemp: ");
      SerialBT.println(String(t3));
    }
  }

}
//EOF