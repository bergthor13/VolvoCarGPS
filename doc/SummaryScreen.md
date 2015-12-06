## Summary Screen
![SummaryScreen](https://raw.githubusercontent.com/bergthor13/VolvoCarGPS/master/doc/images/SummaryScreen.jpg)
Here is the main screen. This screen shows the status of the GPS.
## Speed
Shows the speed in kilometers per hour. The lower number shows acceleration/deceleration.
## Direction
This shows the direction which the car is moving. Both in degrees and N, S, E, W.
## Temperature
The outside temperature as measured by the [Waterproof DS18B20 Digital temperature sensor](https://www.adafruit.com/products/381). It was not connected when the photo was taken.
## Altitude
The calculated altitude of the car.
## Satellites
This shows the number of satellites the GPS is locked onto. Below is the HDOP value (Horizontal Dilution of Precision).
More info [here](https://en.wikipedia.org/wiki/Dilution_of_precision_(GPS)).
## Log and Points
*Log* shows the number of the current file that is being written to on the SD card.

*Points* shows the number of coordinates that have been written to the current file.

## Coordinates
*Acquiring Satellites* means that the GPS is looking for the satellites in the sky. After the GPS has locked onto 3 satellites or more, the coordinates will appear. The GPS will warn you if it loses connection to the satellites.
