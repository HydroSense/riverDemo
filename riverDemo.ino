/*
   Colby Rome 11-13-2015
   */

#include <math.h>
#include <avr/wdt.h>
#include <Wire.h>

#define MINE 3977.0f,298.15f,12000.0f

// Digital IO pins
const byte WSPEED = 19;
const byte RAIN = 18;

// Global variables
long lastSecond;
unsigned minutesSinceLastReset;
byte seconds;
byte seconds_2m;
byte minutes;
byte minutes_10m;

long lastWindCheck = 0;
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;

byte windspdavg[120];
float windspeedmph;
float windspdmph_avg2m;
volatile float dailyrainin;

volatile unsigned long raintime, rainlast, raininterval, rain;

void rainIRQ()
{
    raintime = millis(); // grab current time
    raininterval = raintime - rainlast;
    if(raininterval > 10){ // ignore bounce glitch
        dailyrainin += 0.011;
        rainlast = raintime; 
    }
}

void wspeedIRQ()
{
    if(millis() - lastWindIRQ > 10){
        lastWindIRQ = millis();
        windClicks ++;
    }

}

float get_wind_speed()
{
    float deltaTime = millis() - lastWindCheck;
    deltaTime /= 1000.0;
    float windSpeed = (float)windClicks / deltaTime;
    windClicks = 0;
    lastWindCheck = millis();

    windSpeed *= 1.492;
    return windSpeed;
}


#define SENSOR 3977.0f,298.15f,12000.0f //temperature sensor datasheet

float Temperature(int AnalogInputNumber,float B,float T0,float R0,float R_Balance)
{
  float R,T;

//  R=1024.0f*R_Balance/float(analogRead(AnalogInputNumber)))-R_Balance;
  R=R_Balance*(1024.0f/float(analogRead(AnalogInputNumber))-1);

  T=1.0f/(1.0f/T0+(1.0f/B)*log(R/R0));

  T-=273.15f;

  return T;
}

void setup() 
{
    wdt_reset(); 
    wdt_disable();
    
    Serial.begin(9600);

    pinMode(RAIN, INPUT_PULLUP);
    seconds = 0;
    lastSecond = millis();

    //Attach interrupts
    attachInterrupt(0, rainIRQ, FALLING);
    attachInterrupt(1, wspeedIRQ, FALLING);
    interrupts();

    Serial.println("Weather online!");
}

void loop()
{
    wdt_reset();

    if(millis() - lastSecond >= 1000){
        lastSecond += 1000;
/*
        windspeedmph = get_wind_speed();
        Serial.print("Wind speed = ");
        Serial.println(windspeedmph);
//        Serial.print(windClicks);
        Serial.print("Rain = ");
        Serial.println(dailyrainin);
        Serial.println(Temperature(A3,MINE,10000.0f));
        */
        sendSerial();
    }
    delay(100);
}

void sendSerial()
{
    Serial.print(Temperature(A3,MINE,10000.0f));
    Serial.print('+');
    Serial.print(dailyrainin);
    Serial.print('+');
    Serial.print(get_wind_speed());
    Serial.print('\n');
}

