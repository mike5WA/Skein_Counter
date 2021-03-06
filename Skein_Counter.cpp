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
SCK/D13		to CLK 		P7
MOSI/D11	to MOSI 	P8
D10			to CS/RS 	P9
D9			to RST 		P10
D7 			to D/C 		P11

D2 	(Interrupt 0) to Hall Effect sensor A3213 data pin (3)
D7	to led 22ohm resistor to Grd 

LCD Display = 1.44 128*128 Arduino Module; Driver ST7735
Display 3.3V,
Lcd Module Pins:
P1 VCC; P2 Gnd; P3 Gnd; P4 N/C; P5 N/C; P6 PWM backlight
P7 CLK; P8 SDI/MOSI; P9 RS ; P10 RST; P11 D/C
Code taken from Adafruit "graphicstest.ino" in library

*/

#include <Arduino.h>
#include <Adafruit_GFX.h>    	// Core graphics library
#include <Adafruit_ST7735.h> 	// Hardware-specific library for ST7735
#include <SPI.h>
#include <Adafruit_I2CDevice.h>

#define TFT_CS      10
#define TFT_RST     9 		// Or set to -1 and connect to Arduino RESET pin
#define TFT_DC      6  		//For uno Adafruit used 8 changed to 6 for AT328P feather
#define bLightPin 	5		//Backlight PWM control
#define VBATPIN 	A6		//Analog batvoltage pin

//Using HARDWARE SPI
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const int ledPin = 7;
const int hallPin = 2;		//Interrupt 0
int state = 0;
bool reading = false;		//Monitors if reading taken
bool led = false;			//Monitors if led on or off
int skeinCount =0;			//Variable for revolutions
int oldCount = 0;			//Variable to keep tabs on count changes
int x = 0;
int y = 0;
int size = 1;

uint16_t time = 0;	
unsigned long runtime;
bool isDisplayVisible = false;	//Assume display off until configured
int screenOff = 20;				//No of minutes before screen saver actioned

float meterage = 0;				//Variable for total meterage wound
float metersPerRev = 1;			//Variable for meters wound per revolution
float metersInc = 0.25;			//Variable for incremental increase in meters wound/revolution 

String batText = "";			//Variable for battery status text
int batStatus ;					//Variable to keep tabs on battery status
int batOld = 3;				

//----------------------------------------------------------------------
void revCount ()
//Routine which runs when interrupt 0 triggered on D2 (hallPin)
{
	skeinCount = skeinCount + 1;	//Add 1 to counter
	//Serial.println (skeinCount);
//Blink led on/off as count increments		
	if ((skeinCount % 2) == 0)		//Modulo divide by 2 if even remainder = 0
	{
		digitalWrite(ledPin, HIGH);		//Turn on led
	}
	else
	{
		digitalWrite(ledPin, LOW);		//Turn off led	
	}
}

//------------------------------------------------------------------------
void setup()
{
	Serial.begin(115200);
	pinMode (ledPin, OUTPUT);
	pinMode (hallPin, INPUT);

//Trigger interrupt 0 on falling signal	to run revCount
	attachInterrupt(digitalPinToInterrupt(hallPin), revCount, FALLING);	

	led = false;	//led off at start

	Serial.print(F("ST77xx TFT Test"));
// Initializer for a 1.44" TFT:
  	tft.initR(INITR_144GREENTAB); 		// Init ST7735R chip, green tab
  	Serial.println(F("Initialized"));
  	isDisplayVisible = true;			//Display is on
  	analogWrite(bLightPin, 255);		//Backlight to full

//Set up screen headings  	
  	tft.setRotation(2);					//Set screen portrait
  	tft.fillScreen(ST77XX_BLACK);
  	tft.fillRect(0, 0, 127, 30, ST77XX_WHITE);	//Create white box
  	tft.setTextWrap(false);
//Title
  	tft.setTextSize(3);
  	tft.setCursor(26,5);			//
  	tft.setTextColor(ST77XX_BLUE);
  	tft.println("DATA");
//Meters-Revolution  	
  	tft.setTextSize(2);
  	tft.setTextColor(ST77XX_WHITE);
  	tft.setCursor(0,40);			
  	tft.println("M/R ");
  	tft.setCursor(70,40);			
  	tft.print(metersPerRev,2);
//Revolutions  	
  	tft.setCursor(0,62);			
  	tft.println("Rev ");
//Meterage wound  	
  	tft.setCursor(0,84);
  	tft.println("Mtr ");
//Battery  	
  	tft.setCursor(0,108);			
  	tft.println("Bat ");

  	//time = millis() - time;
  	//Serial.println(time, DEC);
  	delay(500);
}

//------------------------------------------------------------------------
void metersRev ()
//Adjust meters/revolution via button presses from 1m - 2.5m
{
	pinMode(A1, INPUT_PULLUP);			//Pin A1 high (3.3v) taken low (0) if button pressed
	int incrementPin = digitalRead(A1);
//If button pressed increment meters/revolution by .25 between 1 & 2.5
	if (incrementPin == 0)
	{
//Overwrite old data with balack box
		tft.fillRect(50, 36, 77, 20, ST77XX_YELLOW);	
		if (metersPerRev <= 2.25)
		{
			metersPerRev = (metersPerRev + metersInc);
			Serial.print ("Meters per rev =" );
			Serial.println (metersPerRev);
		}
		else 	//Reset to 1
		{
			metersPerRev = (1);
			Serial.print ("Meters per rev =" );
			Serial.println (metersPerRev);
		}
//Display new data
		tft.setCursor(65,40);
		tft.setTextColor(ST77XX_RED);			
  		tft.print(metersPerRev,2);
	}
	delay (150);	//Small delay to avoid bounce
}
//--------------------------------------------------------------------
/*
LiPoly max V ~ 4.2V; Sticks around 3.7v; Cut out ~3.0V
Referance voltage for feather is 3.3v
analogRead will give from 0 to 1023 with 1023 = 3.3v
Feather has double 100K resistor divider on BAT pin connected to A6 so reading is halved
*/

void batVolts ()
{
	float measuredvbat = analogRead(VBATPIN);
//As there are 2 100K resistor/dividers reading is halved
	measuredvbat *= 2;		// multiply by 2 to give true reading
	measuredvbat *= 3.3;  	// Upscale by 3.3V, our reference voltage
	measuredvbat /= 1024; 	// convert to voltage

	//Serial.print("VBat: " ); Serial.println(measuredvbat);
	if (measuredvbat < 3.3)					//Less than operating voltage
	{
		batText = ("Charge");
		tft.setTextColor(ST77XX_RED);
		batStatus = 2;
	}
	if (measuredvbat >= 3.3 && measuredvbat < 3.5)	
	{
		batText = (" Low ");
		tft.setTextColor(ST77XX_ORANGE);
		batStatus = 1;
	}
	if (measuredvbat >= 3.5)
	{
		batText = (" OK ");
		tft.setTextColor(ST77XX_GREEN);	
		batStatus = 0;	
	}
//Refresh battery status only if it has changed to avoid flicker
	if (batStatus != batOld)
	{
//Wipe old data
		tft.fillRect(50, 110, 77, 20, ST77XX_BLACK);	
		tft.setCursor(55,110);
  		tft.println(batText);	
  		batOld = batStatus;
	}
	Serial.print("Battery: " ); Serial.println(measuredvbat);
	Serial.print("Bat Status "); Serial.println(batStatus);
}

//------------------------------------------------------------------------

void dataCalcs ()
{
//If skein counter has changed then recalculate data and display
	
	if (skeinCount != oldCount)
	{
//Display counts & meterage
		runtime = millis();								//Set timer for screensaver
		meterage = (skeinCount * metersPerRev);			//Calculate meterage wound
		tft.fillRect(50, 60, 77, 20, ST77XX_MAGENTA);	//Box to overwrite Rev data
		tft.fillRect(50, 84, 77, 20, ST77XX_BLUE);		//Box to overwrite old meterage
		tft.setCursor(70,62);							//Co-ords of Revolutions data
		tft.setTextColor(ST77XX_WHITE);					//Text colour
		tft.print(skeinCount,DEC);						//Write new skeincount
		tft.setCursor(55,86);							//Co-ords of meterage
		tft.print(meterage,2);							//Write new meterage
		oldCount = skeinCount;
	}
}
//--------------------------------------------------------------------

void counterReset ()
//Reset counter and meterage to zero
{
	pinMode(A0, INPUT_PULLUP);
	int resetPin = digitalRead(A0);
//If button pressed A0 goes low triggering a reset
	if (resetPin == 0)
	{
		skeinCount = 0;
		delay(100);			//Delay to avoid bounce
		dataCalcs ();		//Recalculate sets counter & meterage to 0
	}
}
//--------------------------------------------------------------------

void screenSaver ()
{
//runtime is calculated and stored on every revolution
//If no revolution millis() less runtime will increase x	
//If no rotation for x milliseconds blank screen. 
/*
	Serial.print("millis "); Serial.println(millis());
	Serial.print("Run Time ");	Serial.println(runtime);
	Serial.print("Elapsed Time ");	Serial.println(millis() - runtime);
	delay(500);
*/
	if ((millis() - runtime) > (screenOff * 60000)) 	//No revolution/count for x  milliseconds
	{
//Turn off screen
		isDisplayVisible = false;
		tft.enableDisplay(isDisplayVisible);
//Reduce backlight to 10%
		analogWrite(bLightPin, 25);			
	}
	else 	//runtime <720,000 milliseconds screen on backlight 100%
	{
	isDisplayVisible = true;
	tft.enableDisplay(isDisplayVisible);
	analogWrite(bLightPin, 255);			//Backlight full
	}
}	

//*************************************************************************

void loop()
{
	/* Read state of D2. If low ie triggered interrupt 0 will have been actioned.
		When interrupt actioned revCount() is run and counter incremented see setup.
	*/
	state = digitalRead(hallPin);				//Get status of pin 2 high or low

	if ((state == LOW) && (reading = false))	//Magnet in proximity and no current reading
	{
		reading = true;							//Set true to avoid second reading
	}
	else										//State high no magnet in proximity
	{
		reading = false;						//Set to false to enable another reading
		led = true;				
	}

	counterReset();		//Check if reset button pressed
	metersRev ();		//Check if meters per revolution to be adjusted
	batVolts ();		//Check battery charge
	dataCalcs ();		//Calculate & display data
	screenSaver ();		//Check if no activity & apply screensaver
	delay (200);		//Small delay between readings for stability
}