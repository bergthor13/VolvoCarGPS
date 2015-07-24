#include <SPI.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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
Adafruit_SSD1306  display(OLED_RESET);
OneWire           oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress     tempDeviceAddress;
File logfile;
File fileNumberFile;

bool         gotFix         = false;
boolean      usingInterrupt = false;
unsigned int trkpts         = 0;
unsigned int logs           = 0;

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
        display.invertDisplay(i%2);
        delay(500);
        i++;
    }
}
void setup() {
    display.clearDisplay();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
    display.display();
    Serial. begin(9600);
    sensors.begin();
    pinMode(ledPin, OUTPUT);

    // make sure that the default chip select pin is set to
    // output, even if you don't use it:
    pinMode(10, OUTPUT);
    pinMode(displayButton, OUTPUT);
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect, 11, 12, 13)) {
        Serial.println("Card init. failed!");
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(3);
        display.setCursor(22,4);
        display.println("ERROR");
        display.setTextSize(1);
        display.setCursor(5,30);
        display.println("SD card not found or");
        display.setCursor(25,40);
        display.println("it couldn't be");
        display.setCursor(30,50);
        display.println("initialized");
        display.display();
        error(2);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
    display.setCursor(17,50);
    display.println("Creating file...");
    display.display();

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
            Serial.print("Doesn't exist");
            break;
        }
        logs = i+1;
    }

    logfile = SD.open(filename, FILE_WRITE);
    if(!logfile) {
        Serial.print("Couldnt create ");
        Serial.println(filename);
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(3);
        display.setCursor(22,4);
        display.println("ERROR");
        display.setTextSize(1);
        display.setCursor(30,30);
        display.println(filename);
        display.setCursor(0,40);
        display.println(" couldn't be created");
        display.display();
        error(3);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.drawBitmap(0, 20, volvo, 128, 17, WHITE);
    display.setCursor(21,50);
    display.println("Starting GPS...");
    display.display();
    // connect to the GPS at the desired rate
    GPS.begin(9600);

    // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    // uncomment this line to turn on only the "minimum recommended" data
    //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    // For logging data, we don't suggest using anything but either RMC only or RMC+GGA
    // to keep the log files at a reasonable size
    // Set the update rate
    Serial.println("HERE");
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 100 millihertz (once every 10 seconds), 1Hz or 5Hz update rate
Serial.println("NOT HERE");
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
    if      (round(GPS.angle) < 10)  display.print("00");
    else if (round(GPS.angle) < 100) display.print("0");
    display.print(GPS.angle,0);
}
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

void printTopBar() {
    display.setCursor(1,1);
    display.setTextColor(BLACK, WHITE);
    display.drawLine(0, 0, display.width()-2,0, WHITE);
    display.drawLine(0, 0, 0, 9, WHITE);

    printDate();
    display.print("   ");
    printTime();

    display.setTextColor(WHITE);
    display.drawLine(0, 9, display.width()-2, 9, WHITE);
}
void logPointToFile(DeviceAddress tempSensor) {
    if(logfile.print("20") == 0) {
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(3);
        display.setCursor(22,4);
        display.println("ERROR");
        display.setTextSize(1);
        display.setCursor(14,30);
        display.println("Couldn't write to");
        display.setCursor(9,40);
        display.println("file. Card may have");
        display.setCursor(28,50);
        display.println("been removed.");
        display.display();
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
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(3);
        display.setCursor(22,4);
        display.println("ERROR");
        display.setTextSize(1);
        display.setCursor(14,30);
        display.println("Couldn't write to");
        display.setCursor(9,40);
        display.println("file. Card may have");
        display.setCursor(28,50);
        display.println("been removed.");
        display.display();
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
void updateLogCounter() {
    if      (trkpts < 10)     display.setCursor(97,36);
    else if (trkpts < 100)    display.setCursor(94,36);
    else if (trkpts < 1000)   display.setCursor(92,36);
    else if (trkpts < 10000)  display.setCursor(89,36);
    else if (trkpts < 100000) display.setCursor(87,36);
    display.println(trkpts);
}
void displaySPD() {
    display.drawLine(64, 10, 64, display.height(), WHITE);
    display.drawLine(64, 38, display.width(), 38, WHITE);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,14);
    display.println("SPEED");
    if (GPS.speed*1.852 < 10)  display.print("0");
    if (GPS.speed*1.852 < 100) display.println(GPS.speed*1.852, 2);
    else display.println(GPS.speed*1.852, 1);
    display.setTextSize(1);
    display.println("km/h");

    display.setCursor(70,14);
    display.println("Avg Speed");
    display.setCursor(77,25);
    /*if (maxAlt > 0) {
        if (maxAlt < 10) display.print("0");
        if (maxAlt < 100) display.print(maxAlt, 2);
        else if (maxAlt < 1000) display.print(maxAlt, 1);
        else display.print(maxAlt, 0);
    }
    else {
        display.setCursor(70,25);
        if (maxAlt > -100) display.print(maxAlt, 2);
        else if (maxAlt > -1000) display.print(maxAlt, 1);
        else display.print(maxAlt, 0);
    }*/
    display.println("N/A");


    display.setCursor(70,44);
    display.println("Max Speed");
    display.setCursor(77,55);
    if (maxSpeed > 0) {
        if (maxSpeed < 10) display.print("0");
        if (maxSpeed < 100) display.print(maxSpeed, 2);
        else if (maxSpeed < 1000) display.print(maxSpeed, 1);
        else display.print(maxSpeed, 0);
    }
    else {
        display.setCursor(70,55);
        if (maxSpeed > -100) display.print(maxSpeed, 2);
        else if (maxSpeed > -1000) display.print(maxSpeed, 1);
        else display.print(maxSpeed, 0);
    }
}
void displayGEN() {

    display.setCursor(0,12);
    display.print("SPD: ");
    GPS.speed < 1000 ? display.println(GPS.speed*1.852, 2) : display.println(GPS.speed*1.852, 1);
    display.print("ALT: ");
    GPS.altitude < 1000 ? display.println(GPS.altitude, 2) : display.println(GPS.altitude,1);
    display.print("ANG: ");
    display.println(GPS.angle,2);
    display.print("TMP: ");

    if (currTemp != -3.4028235E+38) {
      display.println(currTemp);
    } else {
      display.println("NO DAT");
    }

    // Lower horizontal line
    display.drawLine(0, 47, display.width(), 47, WHITE);
    display.println();
    // Vertical line
    display.drawLine(70, 10, 70, display.height()-17, WHITE);
    if (GPS.fix) {
        gotFix = true;
        display.print(GPS.lat);
        display.print(" ");
        GPS.lat == 'N' ? display.print(GPS.latitudeDegrees, 4) : display.print((-1)*GPS.latitudeDegrees, 4);
        display.print("   ");
        display.print(GPS.lon);
        display.print(" ");
        GPS.lon == 'W' ? display.print((-1)*GPS.longitudeDegrees, 4) : display.print(GPS.longitudeDegrees, 4);
    }
    else {
        if (!gotFix)
            display.print(" Acquiring Satellites ");
        else
            display.print("   Satellites Lost.   ");
    }
    display.setCursor(75,12);
    display.print("SAT: ");
    display.println((int)GPS.satellites);
    display.setCursor(75,20);
    display.print("DIR: ");
    printDirection();
    display.setCursor(75,28);
    display.print("LOG: ");
    display.println(logs);
    updateLogCounter();
}
void displayTMP(){
    if(sensors.getAddress(tempDeviceAddress, 0)) {
        display.drawLine(64, 10, 64, display.height(), WHITE);
        display.drawLine(64, 38, display.width(), 38, WHITE);
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setCursor(7,14);
        display.println("TEMP");
        display.setCursor(0,32);
        if (currTemp < 10 && currTemp > 0) display.print("0");
        if (currTemp < 0) {
            if (currTemp > -10) display.println(currTemp, 2);
            else if (currTemp > -100) display.println(currTemp, 1);
            else if (currTemp > -1000) display.println(currTemp, 0);
        } else {
            if (currTemp < 100) display.println(currTemp, 2);
            else if (currTemp < 1000) display.println(currTemp, 1);
            else display.println(currTemp, 0);
        }
        display.setTextSize(1);
        display.println("deg. cels.");

        display.setCursor(75,14);
        display.println("Max Temp");
        display.setCursor(77,25);
        if (maxTemp > 0) {
            if (maxTemp < 10) display.print("0");
            if (maxTemp < 100) display.print(maxTemp, 2);
            else if (maxTemp < 1000) display.print(maxTemp, 1);
            else display.print(maxTemp, 0);
        } else {
            display.setCursor(70,25);
            if (maxTemp > -100) display.print(maxTemp, 2);
            else if (maxTemp > -1000) display.print(maxTemp, 1);
            else display.print(maxTemp, 0);
        }
        display.println(" C");

        display.setCursor(75,44);
        display.println("Min Temp");
        display.setCursor(77,55);
        if (minTemp > 0) {
            if (minTemp < 10) display.print("0");
            if (minTemp < 100) display.print(minTemp, 2);
            else if (minTemp < 1000) display.print(minTemp, 1);
            else display.print(minTemp, 0);
        }
        else {
            display.setCursor(70,55);
            if (minTemp > -100) display.print(minTemp, 2);
            else if (minTemp > -1000) display.print(minTemp, 1);
            else display.print(minTemp, 0);
        }
        display.println(" C");
    } else {
        display.setTextSize(1);
        display.setCursor(0,14);
        display.println("Temperature sensor");
        display.println("not connected.");
    }
}
void displayALT(){
    display.drawLine(64, 10, 64, display.height(), WHITE);
    display.drawLine(64, 38, display.width(), 38, WHITE);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(12,14);
    display.println("ALT");
    display.setCursor(0,32);
    if (GPS.altitude < 0) {
        if (GPS.altitude < 10 && GPS.altitude > 0) display.print("0");
        if (GPS.altitude > -1000) display.println(GPS.altitude, 0);
        else if (GPS.altitude > -100) display.println(GPS.altitude, 1);
        else if (GPS.altitude > -10) display.println(GPS.altitude, 2);
    }
    else {
        if (GPS.altitude < 100) display.println(GPS.altitude, 2);
        else if (GPS.altitude < 1000) display.println(GPS.altitude, 1);
        else display.println(GPS.altitude, 0);
    }
    display.setTextSize(1);
    display.println("meters asl");

    display.setCursor(75,14);
    display.println("Max Alt.");
    display.setCursor(77,25);
    if (maxAlt > 0) {
        if (maxAlt < 10) display.print("0");
        if (maxAlt < 100) display.print(maxAlt, 2);
        else if (maxAlt < 1000) display.print(maxAlt, 1);
        else display.print(maxAlt, 0);
    }
    else {
        display.setCursor(70,25);
        if (maxAlt > -100) display.print(maxAlt, 2);
        else if (maxAlt > -1000) display.print(maxAlt, 1);
        else display.print(maxAlt, 0);
    }
    display.println(" m");

    display.setCursor(75,44);
    display.println("Min Alt.");
    display.setCursor(77,55);
    if (minAlt > 0) {
        if (minAlt < 10) display.print("0");
        if (minAlt < 100) display.print(minAlt, 2);
        else if (minAlt < 1000) display.print(minAlt, 1);
        else display.print(minAlt, 0);
    }
    else {
        display.setCursor(70,55);
        if (minAlt > -100) display.print(minAlt, 2);
        else if (minAlt > -1000) display.print(minAlt, 1);
        else display.print(minAlt, 0);
    }
    display.println(" m");
}
void displayANG(){
    display.drawLine(64, 10, 64, display.height(), WHITE);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(2,14);
    display.println("ANGLE");
    display.setCursor(10,40);
    printAngle();
    display.write(9);
    display.setCursor(80,14);
    display.println("DIR");
    display.setCursor(90,40);
    printDirection();
}
void displayDAT(){
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(5,22);
    printDate();
    display.setCursor(17,43);
    printTime();
}
void displayLOC(){
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,14);
    display.setCursor(5,22);
    display.print(GPS.lat);
    display.print(" ");
    GPS.lat == 'N' ? display.print(GPS.latitudeDegrees, 5) : display.print((-1)*GPS.latitudeDegrees, 5);
    display.setCursor(5,43);
    display.print(GPS.lon);
    display.print(" ");
    GPS.lon == 'W' ? display.print((-1)*GPS.longitudeDegrees, 5) : display.print(GPS.longitudeDegrees, 5);
}

void scrollDisplays() {
  if (secs == 10) {
    secs = 0;
    screen++;
    if (screen > 6) {
      screen = 0;
    }
  }

  switch (screen) {
    case 0: displayGEN(); break;
    case 1: displaySPD(); break;
    case 2: displayTMP(); break;
    case 3: displayALT(); break;
    case 4: displayANG(); break;
    case 5: displayLOC(); break;
    case 6: displayDAT(); break;
  }

  secs++;
}

void loop() {
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
        // Refresh the display.
        display.clearDisplay();
        display.setTextSize(1);

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
        printTopBar();
        double potPos = analogRead(0);
        Serial.print (potPos); Serial.print(" - ");
        if (potPos == 1023)                 {display.clearDisplay(); display.display();}
        if (potPos == 0)                    {scrollDisplays();}
        if (potPos > 0   && potPos < 146)    displayGEN();
        if (potPos >= 146 && potPos < 292)   displaySPD();
        if (potPos >= 292 && potPos < 438)   displayTMP();
        if (potPos >= 438 && potPos < 585)   displayALT();
        if (potPos >= 585 && potPos < 731)   displayANG();
        if (potPos >= 731 && potPos < 877)   displayLOC();
        if (potPos >= 877 && potPos < 1023)  displayDAT();

        // Rad. lets log it!
        if (GPS.fix && strstr(stringptr, "RMC")){
            logPointToFile(tempDeviceAddress);
            trkpts++;
        }
        display.display();
    }
}
