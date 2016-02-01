VolvoCarGPS
===========

An Arduino sketch for the GPS in my car.

It logs GPS coordinates to a CSV file every second.

## Components
All components can be bought directly from [www.adafruit.com](http://www.adafruit.com).
- [Arduino Mega 2560 R3](https://www.adafruit.com/products/191)
- [Adafruit Ultimate GPS Logger Shield](https://www.adafruit.com/products/1272) (Replaced with u-blox NEO-M8N)*
- [2.8" TFT LCD with Capacitive Touch](https://www.adafruit.com/products/2090)
- [Waterproof DS18B20 Digital temperature sensor](https://www.adafruit.com/products/381)
- [External Active Antenna - 3-5V 28dB](https://www.adafruit.com/products/960)
- [SMA to uFL/u.FL/IPX/IPEX RF Adapter Cable](https://www.adafruit.com/products/851)

\*I have recently started using the [u-blox NEO-M8N](https://www.u-blox.com/en/product/neo-m8n) GPS module, which supports GPS, GLONASS, BeiDou and the upcoming European system Galileo. It is much more accurate. The branch [`ublox-nmea`](https://github.com/bergthor13/VolvoCarGPS/tree/ublox-nmea) contains the code that works with this module.

I will rewrite the code on branch [`ublox-ubx`](https://github.com/bergthor13/VolvoCarGPS/tree/ublox-ubx) to use the UBX protocol used by the NEO-M8N in the very near future.

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
