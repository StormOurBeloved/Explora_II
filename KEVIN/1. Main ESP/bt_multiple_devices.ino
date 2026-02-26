//BT CO2 & MPL3115A2 module

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;


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
#include <DHT.h>
/*
+ - 3.3V
out - Any (D15)
GND
*/

Adafruit_MPL3115A2 baro;

#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

    //initate sensor
    if (!baro.begin()) {
    Serial.println("Could not find sensor. Check wiring.");
      while(1);   
    }

  // use to set sea level pressure for current location
  // this is needed for accurate altitude measurement
  // STD SLP = 1013.26 hPa
  baro.setSeaPressure(1013.26);
  //initate SCD30 sensor
  scd30.initialize();
  //initate dht sensor
  dht.begin();
}

void loop() {

  delay(750);

  //update sensor values
  //SCD
  float c;
  scd30.getCarbonDioxideConcentration(&c);
  
  //MPL
  float p = baro.getPressure();
  float t = baro.getTemperature();
  float a = baro.getAltitude();

  //dht
  float t2 = dht.readTemperature();
  float h = dht.readHumidity();
  
  delay(250);
  SerialBT.print("P: ");
  SerialBT.print(String(p));
  SerialBT.print("  T: ");
  SerialBT.print(String(t));
  SerialBT.print("  A: ");
  SerialBT.print(String(a));
  SerialBT.print("  H: ");
  SerialBT.print(String(h));
  SerialBT.print("  CO2: ");
  SerialBT.println(String(c));


  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);
}