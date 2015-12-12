#ifndef __SharedFunctions__
#define __SharedFunctions__
#define PI 3.1415926535
#include <WString.h>
#include <Arduino.h>
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <math.h>
#include "Settings.cpp"
#include <avr/sleep.h>

struct Points {
	public:
	int x;
	int y;
};

class SharedFunctions {
private:
	Adafruit_TFTLCD* display;
public:
	String getDirection(double angle) {
		if (angle >= 337 && angle <= 360) return "N";
		if (angle >= 0   && angle < 22)   return "N";
		if (angle >= 67  && angle < 112)  return "E";
		if (angle >= 157 && angle < 202)  return "S";
		if (angle >= 247 && angle < 292)  return "W";
		if (angle >= 22  && angle < 67)   return "NE";
		if (angle >= 112 && angle < 157)  return "SE";
		if (angle >= 202 && angle < 247)  return "SW";
		if (angle >= 292 && angle < 337)  return "NW";
	}

	struct Points getCirclePoint(int angle, int beginX, int beginY, int radius) {

		Points p;

		p.x = beginX + radius * cos((angle * 1000.0 / 57296.0)-(PI/2));
		p.y = beginY + radius * sin((angle * 1000.0 / 57296.0)-(PI/2));

		return p;
	}
	void setDisplay(Adafruit_TFTLCD* display) {
		this->display = display;
	}
};
#endif