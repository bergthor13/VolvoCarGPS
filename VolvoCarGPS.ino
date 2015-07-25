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
#define	GREEN   0x0780
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

#define chipSelect 10
#define ledPin 13
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
	Serial.begin(115200);
	display.reset();
	display.begin(display.readID());
	display.setRotation(3);
	Serial.println("Testing");
	display.fillScreen(BLACK);
	//display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
	//display.display();
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
		//display.clearDisplay();
		//display.setTextColor(WHITE);
		// display.setTextSize(3);
		// display.setCursor(22,4);
		// display.println("ERROR");
		// display.setTextSize(1);
		// display.setCursor(5,30);
		// display.println("SD card not found or");
		// display.setCursor(25,40);
		// display.println("it couldn't be");
		// display.setCursor(30,50);
		// display.println("initialized");
		// display.display();
		error(2);
	}
	// display.clearDisplay();
	// display.setTextSize(1);
	// display.setTextColor(WHITE);
	// display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
	// display.setCursor(17,50);
	// display.println("Creating file...");
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
		// display.clearDisplay();
		// display.setTextColor(WHITE);
		// display.setTextSize(3);
		// display.setCursor(22,4);
		// display.println("ERROR");
		// display.setTextSize(1);
		// display.setCursor(30,30);
		// display.println(filename);
		// display.setCursor(0,40);
		// display.println(" couldn't be created");
		// display.display();
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
	GPS.begin(9600);

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
// Printers.
void printDirection() {
	if(GPS.angle >= 337 && GPS.angle <= 360) display.print("N");
	if(GPS.angle >= 0   && GPS.angle < 22)   display.print("N");
	if(GPS.angle >= 22  && GPS.angle < 67)   display.print("NE");
	if(GPS.angle >= 67  && GPS.angle < 112)  display.print("E");
	if(GPS.angle >= 112 && GPS.angle < 157)  display.print("SE");
	if(GPS.angle >= 157 && GPS.angle < 202)  display.print("S");
	if(GPS.angle >= 202 && GPS.angle < 247)  display.print("SW");
	if(GPS.angle >= 247 && GPS.angle < 292)  display.print("W");
	if(GPS.angle >= 292 && GPS.angle < 337)  display.print("NW");
}
void printAngle() {
	// if      (round(GPS.angle) < 10)  display.print("00");
	// else if (round(GPS.angle) < 100) display.print("0");
	// display.print(GPS.angle,0);
}

struct DisplayDate {
	int sec, min, hr, day, mth, yr;

	DisplayDate(int yr, int mth, int day, int hr, int min, int sec) {
		this->yr  = yr;
		this->mth = mth;
		this->day = day;
		this->hr  = hr;
		this->min = min;
		this->sec = sec;
	}
	void updateDate(int yr, int mth, int day, int hr, int min, int sec) {
		this->yr  = yr;
		this->mth = mth;
		this->day = day;
		this->hr  = hr;
		this->min = min;
		this->sec = sec;
	}
};
DisplayDate newDate(GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);
DisplayDate oldDate(0,0,0,0,0,0);
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
	newDate.updateDate(GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);
	display.setTextColor(BLACK);
	display.setTextSize(2);
	if (asdf == 0)
	{
		display.fillRect(0, 0, 320, 20, GREEN);
		display.drawLine(107, 0, 107, 215, GREEN);
		display.drawLine(213, 0, 213, 215, GREEN);
		display.drawLine(0, 118, 320, 118, GREEN);
		display.drawLine(0, 215, 320, 215, GREEN);
		display.drawLine(250, 215, 250, 240, GREEN);
		display.setTextColor(GREEN);
		display.setCursor(39,30);
		display.setTextSize(1);
		display.print("SPEED");

		display.setCursor(134,30);
		display.setTextSize(1);
		display.print("DIRECTION");

		display.setCursor(234,30);
		display.setTextSize(1);
		display.print("TEMPERATURE");

		display.setCursor(30,128);
		display.setTextSize(1);
		display.print("ALTITUDE");

		display.setCursor(131,128);
		display.setTextSize(1);
		display.print("SATELLITES");

		display.setCursor(258,128);
		display.setTextSize(1);
		display.print("LOG");

		display.setCursor(249,158);
		display.setTextSize(1);
		display.print("POINTS");
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

		if (oldDate.sec != newDate.sec)
		{
			display.fillRect(291,3,10,14, GREEN);
			display.fillRect(303,3,10,14, GREEN);
			display.setCursor(291,3);
			if(GPS.seconds < 10){
				display.print("0");
				display.print(GPS.seconds);
			} else {
				display.print(GPS.seconds);
			}
		}

	}
	display.setCursor(3,3);


	printDate();
	display.print("        ");
	printTime();
	oldDate.updateDate(newDate.yr, newDate.mth, newDate.day, newDate.hr, newDate.min, newDate.sec);
	display.setCursor(3,221);
	display.setTextColor(GREEN);

	if (GPS.fix) {
		gotFix = true;
		display.fillRect(0,216,320,25, BLACK);
		display.print(GPS.lat);
		display.print(" ");
		GPS.lat == 'N' ? display.print(GPS.latitudeDegrees, 4) : display.print((-1)*GPS.latitudeDegrees, 4);
		display.print("  ");
		display.print(GPS.lon);
		display.print(" ");
		GPS.lon == 'W' ? display.print((-1)*GPS.longitudeDegrees, 4) : display.print(GPS.longitudeDegrees, 4);
	}
	else {
			display.fillRect(0,216,320,25, BLACK);
		if (!gotFix)
			display.print(" Acquiring Satellites ");
		else
			display.print("   Satellites Lost.   ");
	}

	display.setCursor(10,64);
	display.setTextSize(3);
	display.print(30.4,1);

	display.setCursor(120,64);
	display.setTextSize(3);
	display.print("NW");

		display.setCursor(240,64);
	display.setTextSize(3);
	display.print(24.3,1);

		display.setCursor(10,170);
	display.setTextSize(3);
	display.print(29.5,1);

		display.setCursor(120,170);
	display.setTextSize(3);
	display.print((int)GPS.satellites);
}

void displayLogsAndPoints() {
	display.fillRect(0,20,320,215, BLACK);
}

void logPointToFile(DeviceAddress tempSensor) {
	if(logfile.print("20") == 0) {
		Serial.println("couldnt log to file!");
		//display.clearDisplay();
		//display.setTextColor(WHITE);
		//display.setTextSize(3);
		//display.setCursor(22,4);
		//display.println("ERROR");
		//display.setTextSize(1);
		//display.setCursor(14,30);
		//display.println("Couldn't write to");
		//display.setCursor(9,40);
		//display.println("file. Card may have");
		//display.setCursor(28,50);
		//display.println("been removed.");
		//display.display();
		error(2);
	}
	logfile.print(GPS.year);
	logfile.print('-');

	if (GPS.month >= 10) {
		logfile.print(GPS.month);
		logfile.print('-');
	}
	else {
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

	logfile.print(GPS.altitude);
	if(logfile.print(';') == 0) {
		Serial.println("ERROR");
		//display.clearDisplay();
		//display.setTextColor(WHITE);
		//display.setTextSize(3);
		//display.setCursor(22,4);
		//display.println("ERROR");
		//display.setTextSize(1);
		//display.setCursor(14,30);
		//display.println("Couldn't write to");
		//display.setCursor(9,40);
		//display.println("file. Card may have");
		//display.setCursor(28,50);
		//display.println("been removed.");
		//display.display();
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
bool hasBeenPressed = false;
void loop() {
	if (ts.touched() && !hasBeenPressed) {
		hasBeenPressed = true;
		//Serial.print("SCREEN TOUCHED AT: ");

		TS_Point p = ts.getPoint();
		// Serial.print(p.x);
		// Serial.print(", ");
		// Serial.println(p.y);
		if (p.x > 20 && p.x < 130 && p.y < 160) {
			Serial.println("UPPER RIGHT");

		} else if (p.x > 130 && p.y < 107) {
			Serial.println("LOWER RIGHT");

		} else if (p.x > 20 && p.x < 130 && p.y > 160) {
			Serial.println("UPPER LEFT");

		} else if (p.x > 130 && p.y > 160) {
			displayLogsAndPoints();
		}
	} else if (!ts.touched()) {
		hasBeenPressed = false;
	}

	if (! usingInterrupt) {
		// read data from the GPS in the 'main loop'
		char c = GPS.read();
		// if you want to debug, this is a good time to do it!
		if (GPSECHO)
			if (c) Serial.print(c);
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

		sensors.requestTemperatures();
		if (sensors.getAddress(tempDeviceAddress, 0)) {
			double temp = sensors.getTempC(tempDeviceAddress);
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
			trkpts++;
		}
		//display.display();
	}
}
