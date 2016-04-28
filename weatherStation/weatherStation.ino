/*
   Colby Rome 11-13-2015
   */

#include <math.h>
#include <Wire.h>

#define MINE 3977.0f,298.15f,10000.0f
#define WDIR A5 // wind vane

// Digital IO pins
const byte WSPEED = 19;
const byte RAIN = 18;
const byte GLED = 9;
const byte BLED = 6;
const byte RLED = 10;

// Global variables
unsigned long lastSecond;
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


#define SENSOR 3977.0f,298.15f,10000.0f //temperature sensor datasheet

float Temperature(int AnalogInputNumber,float B,float T0,float R0,float R_Balance)
{
  float R,T;

//  R=1024.0f*R_Balance/float(analogRead(AnalogInputNumber)))-R_Balance;
  R=R_Balance*(1024.0f/float(analogRead(AnalogInputNumber))-1);

  T=1.0f/(1.0f/T0+(1.0f/B)*log(R/R0));

  T-=273.15f;

  return T;
}

//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
	byte numberOfReadings = 8;
	unsigned int runningValue = 0;

	for(int x = 0 ; x < numberOfReadings ; x++)
		runningValue += analogRead(pinToRead);
	runningValue /= numberOfReadings;

	return(runningValue);
}

int get_wind_direction()
// read the wind direction sensor, return heading in degrees
{
	unsigned int adc;

	adc = averageAnalogRead(WDIR); // get the current reading from the sensor

	// The following table is ADC readings for the wind direction sensor output, sorted from low to high.
	// Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
	// Note that these are not in compass degree order! See Weather Meters datasheet for more information.

	if (adc < 380) return (113);
	if (adc < 393) return (68);
	if (adc < 414) return (90);
	if (adc < 456) return (158);
	if (adc < 508) return (135);
	if (adc < 551) return (203);
	if (adc < 615) return (180);
	if (adc < 680) return (23);
	if (adc < 746) return (45);
	if (adc < 801) return (248);
	if (adc < 833) return (225);
	if (adc < 878) return (338);
	if (adc < 913) return (0);
	if (adc < 940) return (293);
	if (adc < 967) return (315);
	if (adc < 990) return (270);
	return (-1); // error, disconnected?
}

void setup()
{

    Serial.begin(9600);

    Wire.begin(4); // This I2C SLAVE will have address 1.
    Wire.onRequest(sendI2C);

    pinMode(RLED, OUTPUT);
    pinMode(GLED, OUTPUT);
    pinMode(BLED, OUTPUT);
    delay(100);
    digitalWrite(RLED, HIGH);
    //pinMode(RAIN, INPUT_PULLUP);

    seconds = 0;
    lastSecond = millis();

    //Attach interrupts
    //attachInterrupt(digitalPinToInterrupt(11), rainIRQ, FALLING);
    attachInterrupt(digitalPinToInterrupt(7), wspeedIRQ, FALLING);
    //interrupts();

    Serial.println("Weather online!");
}

void loop()
{

    delay(100);
    // while(0 == Wire.available()){
    //     Serial.println("Nothing.");
    //     delay(100);
    // }
    //
    // while(0 != Wire.available()){
    //     Wire.read();
    //     delay(10);
    // }
    // Serial.println("Sending I2C.");
    // sendI2C();`

    // // loop every second
    // if(millis() - lastSecond >= 1000){
    //     lastSecond += 1000;
    //
    //     // windspeedmph = get_wind_speed();
    //     // Serial.print("Wind speed = ");
    //     // Serial.println(windspeedmph);
    //     // Serial.print(windClicks);
    //     // Serial.print("Rain = ");
    //     // Serial.println(dailyrainin);
    //     // Serial.println(Temperature(A3,MINE,10000.0f));
    //
    //     sendSerial();
    // }
}

// Temperature-dailyrain-windspeed
void sendSerial()
{
    Serial.print(Temperature(A4,MINE,10000.0f));
    Serial.print('+');
    Serial.print(get_wind_speed());
    Serial.print('+');
    Serial.print(get_wind_direction());
    Serial.print('\n');
}

void sendI2C()
{
    char buffer[100];
    char str_temp[6];
    char str_temp2[6];
    float temp = Temperature(A4,MINE,10000.0f);
    float windSpeed = get_wind_speed();
    int windDir = get_wind_direction();

    dtostrf(temp, 3, 2, str_temp);
    dtostrf(windSpeed, 3, 2, str_temp2);
    sprintf(buffer, "+%s+%s+%d\n", str_temp, str_temp2, windDir);
    // sprintf(buffer, "+%.2f+%.2f+%d\n",
    //     Temperature(A3,MINE,10000.0f),
    //     get_wind_speed(),
    //     get_wind_direction());

    Serial.print(buffer);
    Wire.print(buffer);

    // Wire.print(Temperature(A4,MINE,10000.0f));
    // Wire.print('+');
    // Wire.print(get_wind_speed());
    // Wire.print('+');
    // Wire.print(get_wind_direction());
    // Wire.print('\n');
}
