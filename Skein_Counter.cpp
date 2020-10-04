/****************************
Mike Garner 20/9/20
Skein counter for skein winder
Increments count per revolution
led turns on & off every revolution
Calculate meterage wound 

Hall Effect Sensor A3213:
P1 = VCC 3.3v; P2 = Grd; P3 = Data/S-Out
47K between Data & 3.3v to pull high
0.1uf cap between VCC & Grd
1.0pf cap between Data & Grd

Adafruit AT328P feather 
A0	to Counter reset button
A1 	to Meters/Revolution adjustment button

D2 	to A3213 data pin (3)
D10	to led 47K resistor to Grd


*/

#include <Arduino.h>
const int ledPin = 10;
const int hallPin = 2;
int state = 0;
bool reading = false;	//Monitors if reading taken
bool led = false;		//Monitors if led on or off
int skeinCount =0;		//Variable for revolutions
float meterage = 0;		//Variable for total meterage wound
float metersPerRev = 1;	//Variable for meters wound per revolution
float metersInc = 0.25;	//Variable for incremental increase in meters wound/revolution 

//----------------------------------------------------------------------
void revCount ()
{
	skeinCount = skeinCount + 1;	//Add 1 to counter
	Serial.print (skeinCount);
	
	if ((skeinCount % 2) == 0)		//Modulo divide by 2 if even remainder = 0
	{
		digitalWrite(ledPin, HIGH);		//Turn on led
	}
	else
	{
		digitalWrite(ledPin, LOW);		//Turn off led	
	}
//Calculate meterage wound 
	meterage = (skeinCount * metersPerRev);
	Serial.print (" meterage = ");
	Serial.println (meterage);		
}

//------------------------------------------------------------------------
void setup()
{
	Serial.begin(115200);
	pinMode (ledPin, OUTPUT);
	pinMode (hallPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(hallPin), revCount, FALLING);	//Will trigger on fall 5v to 0v
	led = false;	//led off
}

//-------------------------------------------------------------------------
void counterReset ()
//Reset counter to zero
{
	pinMode(A0, INPUT_PULLUP);
	int resetPin = digitalRead(A0);
//If button pressed A0 goes low
	if (resetPin == 0)
	{
		skeinCount = 0;
		Serial.print ("Counter reset to ");
		Serial.println (skeinCount);
		delay(150);		//Delay to avoid bounce
	}
}

//------------------------------------------------------------------------
void metersRev ()
//Adjust meters/revolution via button presses from 1m - 2.5m
{
	pinMode(A1, INPUT_PULLUP);
	int incrementPin = digitalRead(A1);
//If button pressed increment meters/revolution by .25 between 1 & 2.5
	if (incrementPin == 0)
	{
		if (metersPerRev <= 2.25)
		{
			metersPerRev = (metersPerRev + metersInc);
			Serial.print ("Meters per rev =" );
			Serial.println (metersPerRev);
		}
		else
		{
			metersPerRev = (1);
			Serial.print ("Meters per rev =" );
			Serial.println (metersPerRev);
		}
	}
	delay (100);	//Small delay to avoid bounce
}

//*************************************************************************
void loop()
{
	
	state = digitalRead(hallPin);	//Get status of pin 2

	if ((state == LOW) && (reading = false))	//Magnet in proximity and no current reading
	{
		revCount ();
		reading = true;			//Set true to avoid second reading
	}
	else	//State high no magnet in proximity
	{
		reading = false;		//Set to false to enable another reading
		led = true;	
	}

	counterReset();		//Check if reset button pressed
	metersRev ();		//Check if meters per revolution to be adjusted
	delay (200);		//Small delay between readings for stability
}