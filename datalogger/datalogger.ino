/*
   Example sketch sending a stream of data to a python script to graph.
   Colby Rome 4-6-16

*/

#include "SDI.h"
#include <Wire.h>

#define SERIAL_OUTPUT_PIN 1
#define FLOW_CONTROL_PIN A3

SDIBusController *SDIBus;
char addr;
unsigned long lastSecond;

void powerSDIMiddlePort(){
    // Powers the middle port on the Hydrosense Datalogger 2.1.2
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);

    Wire.begin();

    // address 0b1110000 (dec 112) refers to the port expander
    Wire.beginTransmission(0b1110000);
    Wire.write(byte(0x03));
    Wire.write(0b00000000);  //Sets all pins to output
    Wire.endTransmission();

    Wire.beginTransmission(0b1110000);
    Wire.write(byte(0x01));
    // Wire.write(0b00000100);  //Sets only Port2On. This is either mislabeled
    // // on the PDF, or incorrectly routed. Pin P2 is on which is incorrectly
    // // called Port3On
    Wire.write(0b1111111);
    Wire.endTransmission();
}

void setup(){
    powerSDIMiddlePort();

    // instantiate SDISerial instance with hardware Serial1
    pinMode(FLOW_CONTROL_PIN, OUTPUT);
    //pinMode(FLOW_CONTROL_PIN, OUTPUT);
    SDISerial *mySDISerial = new SDISerial(Serial1, SERIAL_OUTPUT_PIN, FLOW_CONTROL_PIN);

    // instantiate SDIBus controller, passing in hardware Serial1 as argument
    SDIBus = new SDIBusController(*mySDISerial);

    // Address of Decagon CTD
    addr = '0';

    // For debugging to the computer
    Serial.begin(9600);
    Serial.println("Here it comes");
    lastSecond = millis();
    Wire.begin();




}

void loop(){
    if(millis() - lastSecond >= 1000){
        lastSecond = millis();

        int altno = -1; // 'regular' refreshh function

        // The following will be populated by the call to refresh
        int waitTime;
        int numExpected;

        int res = SDIBus->refresh(addr, altno, &waitTime, &numExpected);
        if(res != 0){
          Serial.print("Received res = ");
          Serial.println(res);
        }
        else{
            delay(500); // theoretically we should wait 1 second.
            float buffer[numExpected];
            res = SDIBus->getData(addr, buffer, numExpected);
            if(res != 0){
              Serial.print("Error during getData. Received: ");
              Serial.println(res);
            }
            else{
              for(int i=0; i<numExpected; i++){
                if(buffer[i] >= 0){
                  Serial.print('+');
                }
                Serial.print(buffer[i]);
              }
              //Serial.print("\r\n");
            }
        }
        Wire.requestFrom(4, 100); // address 4
        // Serial.println("Requested from slave 4.");
        while(0 == Wire.available() && (millis() - lastSecond < 1000)); // wait until a response
        // Serial.println("Past the while loop");

        if(0 != Wire.available()){

            char c = Wire.read();
            while(c != '\n'){
                Serial.print(c);
                c = Wire.read();
            }
            Serial.print('\r');
            Serial.print(c); // should be newline character
        }
    }
    else delay(10);
}
