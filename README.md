VolvoCarGPS
===========

An Arduino sketch for the GPS in my car.

It logs GPS coordinates to a CSV file every second.

## Components
All components can be bought directly from [www.adafruit.com](http://www.adafruit.com).
- [Arduino Mega 2560 R3](https://www.adafruit.com/products/191)
- [Adafruit Ultimate GPS Logger Shield](https://www.adafruit.com/products/1272)
- [2.8" TFT LCD with Capacitive Touch](https://www.adafruit.com/products/2090)
- [Waterproof DS18B20 Digital temperature sensor](https://www.adafruit.com/products/381)
- [External Active Antenna - 3-5V 28dB](https://www.adafruit.com/products/960)
- [SMA to uFL/u.FL/IPX/IPEX RF Adapter Cable](https://www.adafruit.com/products/851)

## Walkthrough of the software
[Summary Screen](https://github.com/bergthor13/VolvoCarGPS/blob/master/doc/SummaryScreen.md)

[Speed Screen](https://github.com/bergthor13/VolvoCarGPS/blob/master/doc/SpeedScreen.md)

Direction Screen

Temperature Screen

Altitude Screen

Satellites Screen

Log and Point Screen

Settings Screen

## The Log File
Here is how the log file is structured. It is a CSV file that contains the following fields:

|Timestamp|Latitude|Longitude|Altitude (m)|Temperature (°C)|
|---|---|---|---|---|
|2015-04-24T15:12:10.0Z|61.04317474365234|-17.96687889099121|22.50|3.44|
|2015-04-24T15:12:10.0Z|61.04317474365234|-17.96687889099121|22.50|NULL|

If the temperature sensor is not connected, `NULL` will be written instead.

I have been preparing a CSV to GPX file converter and will be releasing it shortly.
