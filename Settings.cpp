#ifndef __Settings__
#define __Settings__
// Pins
#define LCD_CS    A3 // Chip Select goes to Analog 3
#define LCD_CD    A2 // Command/Data goes to Analog 2
#define LCD_WR    A1 // LCD Write goes to Analog 1
#define LCD_RD    A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define ONE_WIRE_BUS  31
#define BACKLIGHT_PIN 44
#define CHIPSELECT    10
#define LEDPIN        13
#define DISPLAYBUTTON 40

// Options
#define GPSECHO               false
#define LOG_FIXONLY           true
#define TEMPERATURE_PRECISION 11
#define MAX_SPEED 90

// The size of the screen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Colors
#define BLACK     0x0000
#define BLUE      0x001F
#define VOLVOBLUE 0x001E
#define RED       0xF800
#define GREEN     0xDFEF
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF
#define GREY      0xE71C



#endif