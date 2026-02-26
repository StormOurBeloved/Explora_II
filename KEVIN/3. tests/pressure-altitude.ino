/*
 * RF Transmitter module
 * with:
 *
 * Mission - Pressure/altitude
 * Sensor - BMP280
 * V1
 * 2024-03-21
 * 
 * Vin to Arduino 3.3V pin
 * GND to GND 
 * SCL to Arduino SCL pin (A5) 
 * SDA to Arduino SDA pin (A4)
 *
 */

//Generally important
#include <SPI.h>

//RF24 Libraries
#include "printf.h"
#include "RF24.h"

//module libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>


// instantiate an object for the nRF24L01 transceiver
RF24 radio(7, 8);  // using pin 7 for the CE pin, and pin 8 for the CSN pin

// Let these addresses be used for the pair
uint8_t address[][6] = { "1Node", "2Node" };
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
bool role = true;  // true = TX role, false = RX role
float payload[2] = {0.0, 0.0};

//modeule setup
Adafruit_BMP280 bmp;


void setup() {

  //Serial & RF initialization
  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  // print example's introductory prompt
  Serial.println(F("Explora test board - Transmitter"));

  // To set the radioNumber via the Serial monitor on startup
  //Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
  //while (!Serial.available()) {
    // wait for user input
  //}
  //char input = Serial.parseInt();
  //radioNumber = input == 1;
  Serial.print(F("radioNumber = "));
  Serial.println((int)radioNumber);

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1

  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }

  //Current module
  bmp.begin(115200);
  Serial.println("Adafruit BMP280 test: ");

}  // setup

void loop() {

  if (role) {
    //Get sensor data
    payload[0] = bmp.readPressure();
    payload[1] = bmp.readTemperature();

    // This device is a TX node

    //Transmit
    unsigned long start_timer = micros();                // start the timer
    bool report = radio.write(&payload, sizeof(payload));  // transmit & save the report
    unsigned long end_timer = micros();                  // end the timer

    if (report) {
      Serial.print(F("Transmission successful! "));  // payload was delivered
      Serial.print(F("Time to transmit = "));
      Serial.print(end_timer - start_timer);  // print the timer result
      Serial.print(F(" us. Sent: "));
      Serial.println(payload[0]);  // print payload sent
      //payload += 0.01;          // increment float payload
    } else {
      Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
    }

    // to make this example readable in the serial monitor
    delay(1000);  // slow transmissions down by 1 second

  } else {
    // This device is a RX node

    uint8_t pipe;
    if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      radio.read(&payload, bytes);             // fetch payload from FIFO
      Serial.print(F("Received "));
      Serial.print(bytes);  // print the size of the payload
      Serial.print(F(" bytes on pipe "));
      Serial.print(pipe);  // print the pipe number
      Serial.print(F(": "));
      Serial.println(payload[0]);  // print the payload's value
    }
  }  // role

  if (Serial.available()) {
    // change the role via the serial monitor

    char c = toupper(Serial.read());
  }

}  // loop
