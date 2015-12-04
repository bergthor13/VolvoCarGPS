// Volvo Car GPS with Touch Screen UI
#include <SPI.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <avr/sleep.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_FT6206.h>
#include <math.h>
struct GPS_Status;
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0xDFEF
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define VOLVOGREEN 0x33CC33

#define GPSECHO  false
#define LOG_FIXONLY false
#define OLED_RESET 4
#define ONE_WIRE_BUS 31
#define TEMPERATURE_PRECISION 11
#define BACKLIGHT_PIN 44

#define chipSelect 10
#define ledPin 13

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

float totalDistance = 0;
int screen = 0, secs = 0;
double maxAlt = -3.4028235E+38, minAlt = 3.4028235E+38, maxSpeed = -3.4028235E+38, avgSpeed, maxTemp = -3.4028235E+38, minTemp = 3.4028235E+38, currTemp, currAlt, currSpeed;
bool wasPressed = false;
#define displayButton 40
// LAST UPDATED 26.12.2014

Adafruit_GPS      GPS(&Serial1);
Adafruit_TFTLCD   display(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
Adafruit_FT6206   ts = Adafruit_FT6206();
OneWire           oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress     tempDeviceAddress;
File              logfile;

bool         gotFix         = false;
boolean      usingInterrupt = false;
unsigned int trkpts         = 0;
unsigned int logs           = 0;
int currentScreen           = 0;
bool refresh = true;

static const unsigned char PROGMEM volvo [] = {
    B11111111, B11111111, B11000111, B11111111, B10000011, B11111111, B00000001, B11111111, B11111110, B00111111, B11111111, B11110001,B11111111, B10000011, B11111111, B11110000,
    B11111111, B11111111, B11000111, B11111111, B10011111, B11111111, B11000001, B11111111, B11111110, B00111111, B11111111, B11110001,B11111111, B10011111, B11111111, B11111100,
    B11111111, B11111111, B11000111, B11111111, B10111111, B11111111, B11110001, B11111111, B11111110, B00111111, B11111111, B11110001,B11111111, B10111111, B11111111, B11111110,
    B11111111, B11111111, B11000111, B11111111, B11111111, B11111111, B11111001, B11111111, B11111110, B00111111, B11111111, B11110001,B11111111, B11111111, B11100001, B11111111,
    B00000111, B11111110, B00000001, B11111000, B11111111, B10000111, B11111100, B00011111, B11100000, B00000001, B11111111, B10000000,B01111100, B01111111, B11000001, B11111111,
    B00000111, B11111110, B00000011, B11110001, B11111111, B00000111, B11111100, B00011111, B11100000, B00000001, B11111111, B10000000,B11111100, B01111111, B11000000, B11111111,
    B00000011, B11111111, B00000011, B11110001, B11111111, B00000011, B11111110, B00011111, B11100000, B00000000, B11111111, B11000000,B11111000, B01111111, B11000000, B11111111,
    B00000011, B11111111, B00000111, B11100001, B11111111, B00000011, B11111110, B00011111, B11100000, B00000000, B11111111, B11000001,B11111000, B01111111, B11000000, B11111111,
    B00000001, B11111111, B10000111, B11100001, B11111111, B00000011, B11111110, B00011111, B11100000, B00000000, B01111111, B11100001,B11110000, B01111111, B11000000, B11111111,
    B00000001, B11111111, B11001111, B11000001, B11111111, B00000011, B11111110, B00011111, B11100000, B00011110, B01111111, B11110011,B11100000, B01111111, B11000000, B11111111,
    B00000000, B11111111, B11001111, B10000001, B11111111, B00000011, B11111110, B00011111, B11100000, B00011110, B00111111, B11110011,B11100000, B01111111, B11000000, B11111111,
    B00000000, B01111111, B11111111, B10000001, B11111111, B00000011, B11111110, B00011111, B11100000, B00011110, B00111111, B11111111,B11000000, B00111111, B11100001, B11111111,
    B00000000, B01111111, B11111111, B00000000, B11111111, B10000111, B11111100, B00011111, B11100000, B00111110, B00011111, B11111111,B11000000, B00111111, B11110011, B11111110,
    B00000000, B00111111, B11111111, B00000000, B11111111, B11001111, B11111000, B00011111, B11100000, B01111110, B00001111, B11111111,B10000000, B00011111, B11111111, B11111110,
    B00000000, B00111111, B11111110, B00000000, B01111111, B11111111, B11111001, B11111111, B11111111, B11111110, B00001111, B11111111,B10000000, B00001111, B11111111, B11111000,
    B00000000, B00011111, B11111110, B00000000, B00111111, B11111111, B11100001, B11111111, B11111111, B11111110, B00000111, B11111111,B00000000, B00000011, B11111111, B11100000,
    B00000000, B00011111, B11111100, B00000000, B00001111, B11111111, B10000001, B11111111, B11111111, B11111110, B00000111, B11111111,B00000000, B00000000, B00000000, B00000000};


void useInterrupt(boolean);

uint8_t parseHex(char c) {
	if (c < '0')
		return 0;
	if (c <= '9')
		return c - '0';
	if (c < 'A')
		return 0;
	if (c <= 'F')
		return (c - 'A')+10;
}

// blink out an error code
void error(uint8_t errno) {
	/*
	 if (SD.errorCode()) {
	 putstring("SD error: ");
	 Serial.print(card.errorCode(), HEX);
	 Serial.print(',');
	 Serial.println(card.errorData(), HEX);
	 }
	 */
	int i = 0;
	while(1) {
		//display.invertDisplay(i%2);
		delay(500);
		i++;
	}
}

void setup() {
	//analogWrite(BACKLIGHT_PIN,255);
	Serial.begin(115200);
	display.reset();
	display.begin(display.readID());
	display.setRotation(1);
	display.fillScreen(BLACK);
	//display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
 display.setTextSize(2);
	display.println("Initializing...");
	Serial. begin(115200);
	sensors.begin();
	ts.begin();
	pinMode(ledPin, OUTPUT);

	// make sure that the default chip select pin is set to
	// output, even if you don't use it:
	pinMode(10, OUTPUT);
	pinMode(displayButton, OUTPUT);
	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect, 11, 12, 13)) {
		Serial.println("Card init. failed!");
		display.fillScreen(BLACK);
		display.setTextColor(RED);
		display.setTextSize(5);
		display.setCursor(88,71);
		display.println("ERROR");
		display.setTextSize(2);
		display.setCursor(41,117);
		display.println("SD card not found or");
		display.setCursor(77,136);
		display.println("it couldn't be");
		display.setCursor(89,155);
		display.println("initialized.");
		error(2);
	}
	// display.clearDisplay();
	// display.setTextSize(1);
	// display.setTextColor(WHITE);
	// display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
	// display.setCursor(17,50);
	display.println("Creating file...");
	// display.display();

	char filename[15];
	strcpy(filename, "LOG0000.csv");
	for (uint8_t i = 0; i < 10000; i++) {
		filename[3] = '0' + (i/1000)%10;
		filename[4] = '0' + (i/100)%10;
		filename[5] = '0' + (i/10)%10;
		filename[6] = '0' + i%10;

		// create if does not exist, do not open existing, write, sync after write
		if (! SD.exists(filename)) {
			Serial.print(filename);
			Serial.println(" doesn't exist");
			break;
		}
		logs = i+1;
	}

	logfile = SD.open(filename, FILE_WRITE);
	if(!logfile) {
		Serial.print("Couldnt create ");
		Serial.println(filename);
		display.fillScreen(BLACK);
		display.setTextColor(RED);
		display.setTextSize(5);
		display.setCursor(88,81);
		display.println("ERROR");
		display.setTextSize(2);
		display.setCursor(95,127);
		display.println(filename);
		display.setCursor(47,146);
		display.println("couldn't be created");
		error(3);

	}
	// display.clearDisplay();
	// display.setTextSize(1);
	// display.setTextColor(WHITE);
	// display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
	// display.setCursor(21,50);
	// display.println("Starting GPS...");
	// display.display();
	// connect to the GPS at the desired rate
	display.println("Starting GPS...");
	GPS.begin(9600);
	delay(1000);
	// uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	// uncomment this line to turn on only the "minimum recommended" data
	//GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
	// For logging data, we don't suggest using anything but either RMC only or RMC+GGA
	// to keep the log files at a reasonable size
	// Set the update rate
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 100 millihertz (once every 10 seconds), 1Hz or 5Hz update rate

	// Turn off updates on antenna status, if the firmware permits it
	GPS.sendCommand(PGCMD_NOANTENNA);

	// the nice thing about this code is you can have a timer0 interrupt go off
	// every 1 millisecond, and read data from the GPS for you. that makes the
	// loop code a heck of a lot easier!
	useInterrupt(true);
	display.fillScreen(BLACK);

}
// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
	char c = GPS.read();
	// if you want to debug, this is a good time to do it!
#ifdef UDR0
	if (GPSECHO)
		if (c) UDR0 = c;
	// writing direct to UDR0 is much much faster than Serial.print
	// but only one character can be written at a time.
#endif
}

void useInterrupt(boolean v) {
	if (v) {
		// Timer0 is already used for millis() - we'll just interrupt somewhere
		// in the middle and call the "Compare A" function above
		OCR0A = 0xAF;
		TIMSK0 |= _BV(OCIE0A);
		usingInterrupt = true;
	} else {
		// do not call the interrupt function COMPA anymore
		TIMSK0 &= ~_BV(OCIE0A);
		usingInterrupt = false;
	}
}
//

void printCenteredText(String text, int textSize, int color, int areaWidth, int offset, int y) {
  int x = (areaWidth-(text.length()*textSize*5+textSize*(text.length()-1)))/2+offset;
  display.setTextSize(textSize);
  display.setCursor(x,y);
  display.setTextColor(color);
  display.fillRect(offset+1,y,areaWidth-2,textSize*7,BLACK);
  display.print(text);
}

// Printers.

void printAngle() {
	// if      (round(GPS.angle) < 10)  display.print("00");
	// else if (round(GPS.angle) < 100) display.print("0");
	// display.print(GPS.angle,0);
}

struct DisplayDate {
	int sec, min, hr, day, mth, yr, mil;

	DisplayDate(int yr, int mth, int day, int hr, int min, int sec, int mil) {
		this->yr  = yr;
		this->mth = mth;
		this->day = day;
		this->hr  = hr;
		this->min = min;
		this->sec = sec;
    this->mil = mil;
	}
	void updateDate(int yr, int mth, int day, int hr, int min, int sec, int mil) {
		this->yr  = yr;
		this->mth = mth;
		this->day = day;
		this->hr  = hr;
		this->min = min;
		this->sec = sec;
    this->mil = mil;
	}
};
DisplayDate newDate(GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds);
DisplayDate oldDate(0,0,0,0,0,0,0);
void printDate() {
	if(GPS.day < 10) {
		display.print("0");
		display.print(GPS.day);
	}
	else {
		display.print(GPS.day);
	}

	display.print(".");

	if(GPS.month < 10) {
		display.print("0");
		display.print(GPS.month);
	}
	else {
		display.print(GPS.month);
	}

	display.print(".20");

	if(GPS.year < 10){
		display.print("0");
		display.print(GPS.year, DEC);
	}
	else {
		display.print(GPS.year, DEC);
	}
}
void printTime(){
	if(GPS.hour < 10){
		display.print("0");
		display.print(GPS.hour);
	}
	else {
		display.print(GPS.hour);
	}
	display.print(":");
	if (GPS.minute < 10){
		display.print("0");
		display.print(GPS.minute);
	}
	else {
		display.print(GPS.minute);
	}
	display.print(":");
	if(GPS.seconds < 10){
		display.print("0");
		display.print(GPS.seconds);
	}
	else {
		display.print(GPS.seconds);
	}
}
int asdf = 0;
void printTopBar() {
	newDate.updateDate(GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds);
	display.setTextColor(BLACK);
	display.setTextSize(2);
	if (asdf == 0)
	{
		display.setCursor(3,3);
		display.fillRect(0, 0, 320, 20, GREEN);
		printDate();
		display.print("        ");
		printTime();
		asdf++;
	} else {
		display.setTextSize(2);
		if (oldDate.day != newDate.day)
		{
			display.fillRect(3,3,10,14, GREEN);
			display.fillRect(15,3,10,14, GREEN);
			display.setCursor(3,3);
			if(GPS.day < 10){
				display.print("0");
				display.print(GPS.day);
			} else {
				display.print(GPS.day);
			}
		}

		if (oldDate.mth != newDate.mth)
		{
			display.fillRect(39,3,10,14, GREEN);
			display.fillRect(51,3,10,14, GREEN);
			display.setCursor(39,3);
			if(GPS.month < 10){
				display.print("0");
				display.print(GPS.month);
			} else {
				display.print(GPS.month);
			}
		}


		if (oldDate.yr != newDate.yr)
		{
			display.fillRect(99,3,10,14, GREEN);
			display.fillRect(111,3,10,14, GREEN);
			display.setCursor(99,3);
			if(GPS.year < 10){
				display.print("0");
				display.print(GPS.year, DEC);
			} else {
				display.print(GPS.year, DEC);
			}
		}

		if (oldDate.hr != newDate.hr)
		{
			display.fillRect(219,3,10,14, GREEN);
			display.fillRect(231,3,10,14, GREEN);
			display.setCursor(219,3);
			if(GPS.hour < 10){
				display.print("0");
				display.print(GPS.hour);
			} else {
				display.print(GPS.hour);
			}
		}

		if (oldDate.min != newDate.min)
		{
			display.fillRect(255,3,10,14, GREEN);
			display.fillRect(267,3,10,14, GREEN);
			display.setCursor(255,3);
			if(GPS.minute < 10){
				display.print("0");
				display.print(GPS.minute);
			} else {
				display.print(GPS.minute);
			}
		}

		if (oldDate.sec != newDate.sec || oldDate.mil != newDate.mil)
		{
			display.fillRect(291,3,10,14, GREEN);
			display.fillRect(303,3,10,14, GREEN);
			display.setCursor(291,3);
      int addSecs = GPS.seconds;
      if (GPS.milliseconds >= 500) addSecs++;
			if (addSecs < 10){
				display.print("0");
				display.print(addSecs);
			} else {
				display.print(addSecs);
			}
		}

	}

	oldDate.updateDate(newDate.yr, newDate.mth, newDate.day, newDate.hr, newDate.min, newDate.sec, newDate.mil);
}
struct GPS_Status
{
  int fix;
  double lat, lon;
  GPS_Status(int fix, double lat, double lon) {
    this->fix = fix;
    this->lat = lat;
    this->lon = lon;
  }
  void updateStatus(int fix, double lat, double lon) {
    this->fix = fix;
    this->lat = lat;
    this->lon = lon;
  }
};

GPS_Status newGpsStatus(GPS.fix, GPS.latitudeDegrees, GPS.longitudeDegrees);
GPS_Status oldGpsStatus(-1, 0, 0);

class SummaryScreen {

	double oldSpeed, oldAngle, oldTemperature, oldAltitude, oldAcceleration;
	int oldPoints = -1, oldSatellites;
	String oldDirection;

	String getDirection(double angle) {
		if(angle >= 337 && angle <= 360) return "N";
		if(angle >= 0   && angle < 22)   return "N";
		if(angle >= 67  && angle < 112)  return "E";
		if(angle >= 157 && angle < 202)  return "S";
		if(angle >= 247 && angle < 292)  return "W";
		if(angle >= 22  && angle < 67)   return "NE";
		if(angle >= 112 && angle < 157)  return "SE";
		if(angle >= 202 && angle < 247)  return "SW";
		if(angle >= 292 && angle < 337)  return "NW";
	}

	void displaySpeed(float speed) {
		if (refresh) {
			printCenteredText("SPEED", 1, GREEN, 107, 0, 30);
		}
		double acceleration;
		if (speed != this->oldSpeed || refresh) {
			if (speed < 10)                        printCenteredText(String(speed, 2), 3, GREEN, 107, 0, 64);
			else if (speed >= 10 && speed < 100)   printCenteredText(String(speed, 2), 3, GREEN, 107, 0, 64);
			else if (speed >= 100 && speed < 1000) printCenteredText(String(speed, 1), 3, GREEN, 107, 0, 64);
			else if (speed >= 1000)                printCenteredText(String(speed, 0), 3, GREEN, 107, 0, 64);

			acceleration = speed - this->oldSpeed;
			printCenteredText(String(acceleration, 2), 2, GREEN, 107, 0, 95);
		}

		this->oldSpeed = speed;
		this->oldAcceleration = acceleration;

	}

	void displayDirection(double angle) {

		if (refresh) {
			printCenteredText("DIRECTION", 1, GREEN, 107, 107, 30);
		}

		String dir = this->getDirection(angle);

		if (this->oldDirection != dir || refresh) {
			printCenteredText(dir, 3, GREEN, 107, 107, 64);
		}
		this->oldDirection = dir;

		if (this->oldAngle != angle || refresh) {
			printCenteredText(String(angle), 2, GREEN, 107, 107, 95);
		}
		this->oldAngle = angle;
	}

	void displayTemperature(double temp) {
		if (refresh) {
			printCenteredText("TEMPERATURE", 1, GREEN, 107, 214, 30);
		}

		if (this->oldTemperature != temp || refresh) {

			if (currTemp != -3.4028235E+38) {
				if      (temp <= -100 || temp >= 1000)   printCenteredText(String(temp, 0), 3, GREEN, 107, 214, 64);
				else if ((temp <= -10 && temp > -100) ||
				         (temp >= 100 && temp < 1000))   printCenteredText(String(temp, 1), 3, GREEN, 107, 214, 64);
				else if (-10 < temp && temp < 100)       printCenteredText(String(temp, 2), 3, GREEN, 107, 214, 64);
			} else {
				printCenteredText("DISC", 3, GREEN, 107, 214, 64);
			}
		}
		this->oldTemperature = temp;
	}

	void displayAltitude(double alt) {
		if (refresh) {
			display.setCursor(30,128);
			display.setTextSize(1);
			display.print("ALTITUDE");
		}
		if (this->oldAltitude != alt || refresh) {
			if      (alt <= -100 || alt >= 1000)   printCenteredText(String(alt, 0), 3, GREEN, 107, 214, 162);
			else if ((alt <= -10 && alt > -100) ||
			         (alt >= 100 && alt < 1000))   printCenteredText(String(alt, 1), 3, GREEN, 107, 0, 162);
			else if (-10 < alt && alt < 100)       printCenteredText(String(alt, 2), 3, GREEN, 107, 0, 162);
		}
		this->oldAltitude = alt;
	}

	void displaySatellites(int sat) {
		if (refresh) {
			display.setCursor(131,128);
			printCenteredText("SATELLITES", 1, GREEN, 107, 107, 128);
		}

		if (this->oldSatellites != sat || refresh) {
			printCenteredText(String(sat), 3, GREEN, 107, 107, 162);
		}
		this->oldSatellites = sat;
	}

	void displayLogsAndPoints(int logNumber, int point) {
		if (refresh) {
			printCenteredText("LOG", 1, GREEN, 107, 214, 128);
			printCenteredText("POINTS", 1, GREEN, 107, 214, 174);
			printCenteredText(String(logNumber), 2, GREEN, 107, 214, 142);
		}

		if (this->oldPoints != point || refresh) {
			printCenteredText(String(point), 2, GREEN, 107, 214, 188);
		}
		this->oldPoints = point;
	}

	void displayOutlines() {
		if (refresh) {
			display.drawLine(107,20, 107, 215, GREEN);
			display.drawLine(213,20, 213, 215, GREEN);
			display.drawLine(0, 118, 320, 118, GREEN);
			display.drawLine(0, 215, 320, 215, GREEN);
		}
	}

	void displayGPSStatus(int fix, double lat, double lon) {
		newGpsStatus.updateStatus(GPS.fix, GPS.latitudeDegrees, GPS.longitudeDegrees);
		display.setTextColor(GREEN);
		display.setTextSize(2);

		if (GPS.fix) {
			display.setCursor(35,221);
			gotFix = true;
			if (newGpsStatus.lat != oldGpsStatus.lat ||
				newGpsStatus.lon != oldGpsStatus.lon || refresh) {

				display.fillRect(35,221,250,16, BLACK);
				display.print(GPS.lat);
				display.print(" ");
				GPS.lat == 'N' ? display.print(GPS.latitudeDegrees, 4) : display.print((-1)*GPS.latitudeDegrees, 4);
				display.print("   ");
				display.print(GPS.lon);
				display.print(" ");
				GPS.lon == 'W' ? display.print((-1)*GPS.longitudeDegrees, 4) : display.print(GPS.longitudeDegrees, 4);
			}
		}
		else {

			if (newGpsStatus.fix != oldGpsStatus.fix || refresh)
			{
				display.fillRect(0,216,320,25, BLACK);
				if (!gotFix) {
					display.setCursor(41,221);
					display.setTextColor(BLUE);
					display.print("Acquiring Satellites");
				} else {
					display.setCursor(71,221);
					display.setTextColor(RED);
					display.print("Lost Satellites");
				}
			}

		}
		oldGpsStatus.updateStatus(newGpsStatus.fix, newGpsStatus.lat, newGpsStatus.lon);
	}
	public:
	void displayScreen() {
		displayOutlines();
		displaySpeed(GPS.speed*1.852);
		displayDirection(GPS.angle);
		displayTemperature(currTemp);
		displayAltitude(GPS.altitude);
		displaySatellites(GPS.satellites);
		displayLogsAndPoints(logs,trkpts);
		displayGPSStatus(GPS.fix, GPS.latitudeDegrees, GPS.longitudeDegrees);
		refresh = false;
	}
};

SummaryScreen summaryScreen;
float oldMappedSpeed = NULL;
float oldMappedMaxSpeed = NULL;
float oldMappedAvgSpeed = 40;

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
int oldMaxSpeed = 999;

//TODO: Refactor this into a class like SummaryScreen.
void displaySpeedScreen() {
	if (refresh)
	{
		// Print out the data cells titles
		printCenteredText("SPEED", 1, GREEN, 80, 0, 24);
		printCenteredText("AVG. SPEED", 1, GREEN, 79, 80, 24);
		printCenteredText("MAX SPEED", 1, GREEN, 79, 160, 24);
		printCenteredText("DISTANCE", 1, GREEN, 80, 240, 24);

		printCenteredText("ALTITUDE", 1, GREEN, 80, 0, 65);
		printCenteredText("SATELLITES", 1, GREEN, 80, 240, 65);

		display.drawLine(0,60,340,60,GREEN);
		display.drawLine(0,100,80,100,GREEN);
		display.drawLine(240,100,320,100,GREEN);

		display.drawLine(80,20,80,100,GREEN);
		display.drawLine(160,20,160,60,GREEN);
		display.drawLine(240,20,240,100,GREEN);

		display.drawCircle(160,240,150,GREEN);
		display.drawLine(21, 188, 33, 193, GREEN);
		display.drawLine(47, 144, 57, 152, GREEN);
		display.drawLine(86, 111, 92, 122, GREEN);
		display.drawLine(134, 93, 137, 106, GREEN);
		display.drawLine(186, 93, 183, 106, GREEN);
		display.drawLine(234, 111, 227, 122, GREEN);
		display.drawLine(273, 144, 263, 152, GREEN);
		display.drawLine(299, 188, 287, 193, GREEN);

		display.drawLine(14, 213, 22, 215, GREEN);
		display.drawLine(32, 165, 39, 169, GREEN);
		display.drawLine(65, 126, 70, 132, GREEN);
		display.drawLine(109, 100, 112, 107, GREEN);
		display.drawLine(160, 91, 160, 99, GREEN);
		display.drawLine(211, 100, 208, 107, GREEN);
		display.drawLine(255, 126, 250, 132, GREEN);
		display.drawLine(288, 165, 281, 169, GREEN);
		display.drawLine(306, 213, 298, 215, GREEN);

		display.drawLine(12, 234, 15, 234, GREEN);
		display.drawLine(12, 229, 15, 229, GREEN);
		display.drawLine(13, 224, 16, 224, GREEN);
		display.drawLine(13, 218, 16, 219, GREEN);
		display.drawLine(15, 208, 18, 209, GREEN);
		display.drawLine(16, 203, 19, 204, GREEN);
		display.drawLine(18, 198, 21, 199, GREEN);
		display.drawLine(19, 193, 22, 194, GREEN);
		display.drawLine(23, 184, 26, 185, GREEN);
		display.drawLine(25, 179, 28, 180, GREEN);
		display.drawLine(27, 174, 30, 175, GREEN);
		display.drawLine(29, 170, 32, 171, GREEN);
		display.drawLine(34, 161, 37, 162, GREEN);
		display.drawLine(37, 156, 40, 158, GREEN);
		display.drawLine(40, 152, 43, 154, GREEN);
		display.drawLine(43, 148, 46, 150, GREEN);
		display.drawLine(50, 140, 52, 142, GREEN);
		display.drawLine(54, 136, 56, 138, GREEN);
		display.drawLine(57, 133, 59, 135, GREEN);
		display.drawLine(61, 129, 63, 131, GREEN);
		display.drawLine(69, 122, 71, 125, GREEN);
		display.drawLine(73, 119, 75, 122, GREEN);
		display.drawLine(77, 116, 79, 119, GREEN);
		display.drawLine(82, 113, 83, 116, GREEN);
		display.drawLine(91, 108, 92, 111, GREEN);
		display.drawLine(95, 106, 96, 109, GREEN);
		display.drawLine(100, 104, 101, 107, GREEN);
		display.drawLine(105, 102, 106, 105, GREEN);
		display.drawLine(114, 98, 115, 101, GREEN);
		display.drawLine(119, 97, 120, 100, GREEN);
		display.drawLine(124, 95, 125, 98, GREEN);
		display.drawLine(129, 94, 130, 97, GREEN);
		display.drawLine(139, 92, 140, 95, GREEN);
		display.drawLine(145, 92, 145, 95, GREEN);
		display.drawLine(150, 91, 150, 94, GREEN);
		display.drawLine(155, 91, 155, 94, GREEN);
		display.drawLine(165, 91, 165, 94, GREEN);
		display.drawLine(170, 91, 170, 94, GREEN);
		display.drawLine(175, 92, 175, 95, GREEN);
		display.drawLine(181, 92, 180, 95, GREEN);
		display.drawLine(191, 94, 190, 97, GREEN);
		display.drawLine(196, 95, 195, 98, GREEN);
		display.drawLine(201, 97, 200, 100, GREEN);
		display.drawLine(206, 98, 205, 101, GREEN);
		display.drawLine(215, 102, 214, 105, GREEN);
		display.drawLine(220, 104, 219, 107, GREEN);
		display.drawLine(225, 106, 224, 109, GREEN);
		display.drawLine(229, 108, 228, 111, GREEN);
		display.drawLine(238, 113, 237, 116, GREEN);
		display.drawLine(243, 116, 241, 119, GREEN);
		display.drawLine(247, 119, 245, 122, GREEN);
		display.drawLine(251, 122, 249, 125, GREEN);
		display.drawLine(259, 129, 257, 131, GREEN);
		display.drawLine(263, 133, 261, 135, GREEN);
		display.drawLine(266, 136, 264, 138, GREEN);
		display.drawLine(270, 140, 268, 142, GREEN);
		display.drawLine(277, 148, 274, 150, GREEN);
		display.drawLine(280, 152, 277, 154, GREEN);
		display.drawLine(283, 156, 280, 158, GREEN);
		display.drawLine(286, 161, 283, 162, GREEN);
		display.drawLine(291, 170, 288, 171, GREEN);
		display.drawLine(293, 174, 290, 175, GREEN);
		display.drawLine(295, 179, 292, 180, GREEN);
		display.drawLine(297, 184, 294, 185, GREEN);
		display.drawLine(301, 193, 298, 194, GREEN);
		display.drawLine(302, 198, 299, 199, GREEN);
		display.drawLine(304, 203, 301, 204, GREEN);
		display.drawLine(305, 208, 302, 209, GREEN);
		display.drawLine(307, 218, 304, 219, GREEN);
		display.drawLine(307, 224, 304, 224, GREEN);
		display.drawLine(308, 229, 305, 229, GREEN);
		display.drawLine(308, 234, 305, 234, GREEN);
	}

	// Print out the data cells data.
	printCenteredText(String(GPS.speed*1.852,2), 2, GREEN, 80, 0, 38);
	printCenteredText(String(avgSpeed,2), 2, GREEN, 80, 80, 38);
	printCenteredText(String(maxSpeed,2), 2, GREEN, 80, 160, 38);
	printCenteredText(String(totalDistance,2), 2, GREEN, 80, 240, 38);

	printCenteredText(String(GPS.altitude,2), 2, GREEN, 80, 0, 79);
	printCenteredText(String(GPS.satellites), 2, GREEN, 80, 240, 79);

	float mappedSpeed    = mapfloat(GPS.speed*1.852, 0.0, 90.0, 180.0, 360.0);
	float mappedMaxSpeed = mapfloat(maxSpeed, 0.0, 90.0, 180.0, 360.0);
	float mappedAvgSpeed = mapfloat(avgSpeed, 0.0, 90.0, 180.0, 360.0);
	if(mappedSpeed < 180) mappedSpeed = 180;

	display.setTextSize(1);
	display.setTextColor(GREEN);

	display.setCursor(40, 194); display.print(10);
	display.setCursor(62, 157); display.print(20);
	display.setCursor(92, 128); display.print(30);
	display.setCursor(133, 113); display.print(40);
	display.setCursor(176, 113); display.print(50);
	display.setCursor(217, 128); display.print(60);
	display.setCursor(247, 157); display.print(70);
	display.setCursor(269, 194); display.print(80);
	display.drawLine(160,
	                 239,
	                 (160 + (130 * cos(oldMappedSpeed * 1000.0 / 57296.0))),
	                 (239 + (130 * sin(oldMappedSpeed * 1000.0 / 57296.0))),
	                 BLACK);
	display.drawLine(160,239, (160 + (130 * cos(mappedSpeed * 1000.0 / 57296.0))), (239 + (130 * sin(mappedSpeed * 1000.0 / 57296.0))), RED);

	display.drawLine((160 + (160 * cos(oldMappedMaxSpeed * 1000.0 / 57296.0))),
	                 (239 + (160 * sin(oldMappedMaxSpeed * 1000.0 / 57296.0))),
	                 (160 + (151 * cos(oldMappedMaxSpeed * 1000.0 / 57296.0))),
	                 (239 + (151 * sin(oldMappedMaxSpeed * 1000.0 / 57296.0))),
	                 BLACK);
	display.drawLine((160 + (160 * cos(mappedMaxSpeed * 1000.0 / 57296.0))),
	                 (239 + (160 * sin(mappedMaxSpeed * 1000.0 / 57296.0))),
	                 (160 + (151 * cos(mappedMaxSpeed * 1000.0 / 57296.0))),
	                 (239 + (151 * sin(mappedMaxSpeed * 1000.0 / 57296.0))),
	                 RED);

	display.drawLine((160 + (160 * cos(oldMappedAvgSpeed * 1000.0 / 57296.0))),
	                 (239 + (160 * sin(oldMappedAvgSpeed * 1000.0 / 57296.0))),
	                 (160 + (151 * cos(oldMappedAvgSpeed * 1000.0 / 57296.0))),
	                 (239 + (151 * sin(oldMappedAvgSpeed * 1000.0 / 57296.0))),
	                 BLACK);
	display.drawLine((160 + (160 * cos(mappedAvgSpeed * 1000.0 / 57296.0))),
	                 (239 + (160 * sin(mappedAvgSpeed * 1000.0 / 57296.0))),
	                 (160 + (151 * cos(mappedAvgSpeed * 1000.0 / 57296.0))),
	                 (239 + (151 * sin(mappedAvgSpeed * 1000.0 / 57296.0))),
	                 BLUE);

	oldMappedMaxSpeed = mappedMaxSpeed;
	oldMappedAvgSpeed = mappedAvgSpeed;
	display.drawBitmap(96, 170, volvo, 128, 17, BLUE);
	oldMappedSpeed = mappedSpeed;
	refresh = false;
}

void displayDirectionScreen() {
	if (refresh)
	{
		display.setTextSize(2);
		display.setTextColor(GREEN);
		display.setCursor(3,23);
		display.print("Direction:");
		display.drawCircle(160,130,100,GREEN);
		display.drawLine(259,  130, 250, 130, GREEN);
		display.drawLine(230,  200, 224, 194, GREEN);
		display.drawLine(160,  229, 160, 220, GREEN);
		display.drawLine(90,   200, 96,  194, GREEN);
		display.drawLine(61,   130, 70,  130, GREEN);
		display.drawLine(90,   60,  96,  66,  GREEN);
		display.drawLine(160,  31,  160, 40,  GREEN);
		display.drawLine(230,  60,  224, 66,  GREEN);
		display.drawLine(259,  130, 250, 130, GREEN);

		display.drawLine(160,
		                 130,
		                 (160 + (90 * cos((GPS.angle * 1000.0 / 57296.0)-(PI/2)))),
		                 (130 + (90 * sin((GPS.angle * 1000.0 / 57296.0)-(PI/2)))),
		                 RED);
	}
	refresh = false;
}

void displayTemperatureScreen() {
	if (refresh)
	{
		display.setTextSize(2);
		display.setTextColor(GREEN);
		display.setCursor(3,23);
		display.print("Temperature:\nWork in progress...");
	}
	refresh = false;
}

void displayAltitudeScreen() {
	if (refresh)
	{
		display.setTextSize(2);
		display.setTextColor(GREEN);
		display.setCursor(3,23);
		display.print("Altitude:\nWork in progress...");
	}
	refresh = false;
}

void displaySatellitesScreen() {
	if (refresh)
	{
		display.setTextSize(2);
		display.setTextColor(GREEN);
		display.setCursor(3,23);
		display.print("Satellites:\nWork in progress...");
	}
	refresh = false;
}

void displayLogsAndPoints() {
	if (refresh)
	{
		display.drawLine(0,130,320,130,GREEN);
		display.setTextSize(2);
		display.setTextColor(GREEN);
		display.setCursor(3,23);
		display.print("Time and position:\nWork in progress...");
	}
	refresh = false;
}

void logPointToFile(DeviceAddress tempSensor) {
	if(logfile.print("20") == 0) {
		Serial.println("couldnt log to file!");
		display.fillScreen(BLACK);

		printCenteredText("ERROR", 5, RED, 320, 0, 71);
		printCenteredText("Couldn't write to", 2, RED, 320, 0, 117);
		printCenteredText("file. Card may have", 2, RED, 320, 0, 136);
		printCenteredText("been removed.", 2, RED, 320, 0, 155);
		error(2);
	}
	logfile.print(GPS.year);
	logfile.print('-');

	if (GPS.month >= 10) {
		logfile.print(GPS.month);
		logfile.print('-');
	} else {
		logfile.print('0');
		logfile.print(GPS.month);
		logfile.print('-');
	}

	if (GPS.day >= 10)    {
		logfile.print(GPS.day);
	} else {
		logfile.print('0');
		logfile.print(GPS.day);
	}

	logfile.print('T');

	if (GPS.hour >= 10)    {
		logfile.print(GPS.hour), logfile.print(':');
	}
	else {
		logfile.print('0'), logfile.print(GPS.hour), logfile.print(':');
	}

	if (GPS.minute >= 10)    {
		logfile.print(GPS.minute), logfile.print(':');
	}
	else {
		logfile.print('0'), logfile.print(GPS.minute), logfile.print(':');
	}

	if (GPS.seconds >= 10)    {
		logfile.print(GPS.seconds), logfile.print('.');
	}
	else {
		logfile.print('0'), logfile.print(GPS.seconds), logfile.print('.');
	}

	logfile.print(GPS.milliseconds);
	logfile.print('Z');
	logfile.print(';');

	logfile.print(GPS.latitudeDegrees,14);
	logfile.print(';');

	logfile.print(GPS.longitudeDegrees,14);
	logfile.print(';');

	logfile.print(GPS.altitude,14);
	if(logfile.print(';') == 0) {
		Serial.println("ERROR");
		display.fillScreen(BLACK);
		printCenteredText("ERROR",               5, RED, 320, 0, 71);
		printCenteredText("Couldn't write to",   2, RED, 320, 0, 117);
		printCenteredText("file. Card may have", 2, RED, 320, 0, 136);
		printCenteredText("been removed.",       2, RED, 320, 0, 155);

		error(2);
	}
	if(sensors.getAddress(tempDeviceAddress, 0)) {
	  double temp = sensors.getTempC(tempSensor);
	  if (temp < minTemp) minTemp = temp;
	  if (temp > maxTemp) maxTemp = temp;
	  logfile.println(temp);
	}
	else {
		logfile.println("NULL");
	}
	logfile.flush();
}
double distanceBetweenPoints(double lat1, double lat2, double lon1, double lon2) {
    int R = 6371; // km
    double dLat = (lat2-lat1)*PI/180;
    double dLon = (lon2-lon1)*PI/180;
    lat1 = lat1*PI/180;
    lat2 = lat2*PI/180;

    double a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = R * c;

    return d;
}

bool hasBeenPressed = false;
double oldLat = NULL, oldLon = NULL;

bool tappedSpeed(int x, int y) { return 118 < x && x <= 215 && 107 >= y && y > 0;   }
bool tappedDir  (int x, int y) { return 118 < x && x <= 215 && 213 >= y && y > 107; }
bool tappedTemp (int x, int y) { return 118 < x && x <= 215 && 320 >= y && y > 213; }

bool tappedAlt  (int x, int y) { return 20 < x && x <= 118 && 107 >= y && y >= 0;   }
bool tappedSats (int x, int y) { return 20 < x && x <= 118 && 213 >= y && y > 107;  }
bool tappedLogs (int x, int y) { return 20 < x && x <= 118 && 320 >= y && y > 213;  }

bool tappedTime (int x, int y) { return false; }

void loop() {
	if (ts.touched() && !hasBeenPressed) {
		hasBeenPressed = true;

		TS_Point p = ts.getPoint();
		Serial.print("(");
		Serial.print(p.x);
		Serial.print(", ");
		Serial.print(p.y);
		Serial.println(")");

		if (currentScreen != 0)
		{
			display.fillRect(0,20,320,240, BLACK);
			currentScreen = 0;
			refresh = true;
		} else {
			if (tappedSpeed(p.x, p.y)) {
				display.fillRect(0,20,320,240, BLACK);
				currentScreen = 1;
				refresh = true;
			} else if (tappedDir(p.x, p.y)) {
				display.fillRect(0,20,320,240, BLACK);
				currentScreen = 2;
				refresh = true;
			} else if (tappedTemp(p.x, p.y)) {
				display.fillRect(0,20,320,240, BLACK);
				currentScreen = 3;
				refresh = true;
			} else if (tappedAlt(p.x, p.y)) {
				display.fillRect(0,20,320,240, BLACK);
				currentScreen = 4;
				refresh = true;
			} else if (tappedSats(p.x, p.y)) {
				display.fillRect(0,20,320,240, BLACK);
				currentScreen = 5;
				refresh = true;
			} else if (tappedLogs(p.x, p.y)) {
				display.fillRect(0,20,320,240, BLACK);
				currentScreen = 6;
				refresh = true;
			}
		}

	} else if (!ts.touched()) {
		hasBeenPressed = false;
	}


	// if a sentence is received, we can check the checksum, parse it...
	if (GPS.newNMEAreceived()) {
		// a tricky thing here is if we print the NMEA sentence, or data
		// we end up not listening and catching other sentences!
		// so be very wary if using OUTPUT_ALLDATA and trying to print out data

		// Don't call lastNMEA more than once between parse calls!  Calling lastNMEA
		// will clear the received flag and can cause very subtle race conditions if
		// new data comes in before parse is called again.

		char *stringptr = GPS.lastNMEA();

		if (!GPS.parse(stringptr))   // this also sets the newNMEAreceived() flag to false
			return;  // we can fail to parse a sentence in which case we should just wait for another

		// Sentence parsed!
		if (LOG_FIXONLY && !GPS.fix) {
			return;
		}
		// TODO: Refresh the display.
		printTopBar();
		switch (currentScreen) {
			case 0:
				summaryScreen.displayScreen();
				break;
			case 1:
				displaySpeedScreen();
				break;
			case 2:
				displayDirectionScreen();
				break;
			case 3:
				displayTemperatureScreen();
				break;
			case 4:
				displayAltitudeScreen();
				break;
			case 5:
				displaySatellitesScreen();
				break;
			case 6:
				displayLogsAndPoints();
				break;
		}



		sensors.requestTemperatures();
		double temp;
		if (sensors.getAddress(tempDeviceAddress, 0)) {
			temp = sensors.getTempC(tempDeviceAddress);
			currTemp = temp;
			if (temp < minTemp) minTemp = temp;
			if (temp > maxTemp) maxTemp = temp;
		} else {
			currTemp = -3.4028235E+38;
		}
		currAlt = GPS.altitude;
		if (currAlt < minAlt) minAlt = currAlt;
		if (currAlt > maxAlt) maxAlt = currAlt;

		currSpeed = GPS.speed*1.852;
		if (currSpeed > maxSpeed) maxSpeed = currSpeed;
		// UPDATE DATE AND TIME
		// display.fillScreen(BLACK);
		// printTopBar();
		double potPos = analogRead(0);
		//Serial.print (potPos); Serial.print(" - ");
		//if (potPos == 1023)                 {display.clearDisplay(); display.display();}

		// Rad. lets log it!
		if (GPS.fix && strstr(stringptr, "RMC")){
			logPointToFile(tempDeviceAddress);
			if(oldLat != NULL && oldLon != NULL) {
				totalDistance += distanceBetweenPoints(oldLat,GPS.latitudeDegrees,oldLon,GPS.longitudeDegrees);
				// This will do for now.
				avgSpeed = totalDistance/trkpts*60*60;
			}
			oldLat = GPS.latitudeDegrees;
			oldLon = GPS.longitudeDegrees;

			trkpts++;
		}
	}
}
