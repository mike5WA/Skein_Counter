/****************************
Mike Garner 20/9/20
Skein counter for skein winder
Increments count per revolution
led turns on & off every revolution

*/
#include <Arduino.h>
const int ledPin = 10;
const int hallPin = 2;
int state = 0;
bool reading = false;	//Monitors if reading taken
bool led = false;		//Monitors if led on or off
int skeinCount =0;		//Variable for revolutions

void revCount ()
{
	skeinCount = skeinCount + 1;	//Add 1 to counter
	Serial.println (skeinCount);
	
	if ((skeinCount % 2) == 0)		//Modulo divide by 2 if even remainder = 0
	{
		digitalWrite(ledPin, HIGH);		//Turn on led
	}
	else
	{
		digitalWrite(ledPin, LOW);		//Turn off led	
	}
		
}

void setup()
{
	Serial.begin(115200);
	pinMode (ledPin, OUTPUT);
	pinMode (hallPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(hallPin), revCount, FALLING);	//Will trigger on fall 5v to 0v
	led = false;	//led off
}


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
	
	delay (100);		//Small delay between readings for stability
}