/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

Pick one up today in the adafruit shop!
------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using SPI to communicate
4 or 5 pins are required to interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#include <stdio.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS1302.h>
#include <RF24.h>

const uint64_t RF_ADDR = 0xFEEDFEED00LL;
const int RF_CHANNEL = 12;
const int RF_PAYLOAD = 32;
const int RF_PIN_CE  =  9;
const int RF_PIN_SS  = 10;
RF24 radio(RF_PIN_CE, RF_PIN_SS);

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
const int kCePin   = 2;  // Chip Enable
const int kIoPin   = 3;  // Input/Output
const int kSclkPin = 4;  // Serial Clock

// Create a DS1302 object.
DS1302 rtc(kCePin, kIoPin, kSclkPin);

// If using software SPI (the default case):
#define OLED_MOSI  6
#define OLED_CLK   5
#define OLED_DC    8
#define OLED_CS    20
#define OLED_RESET 7
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

/* Uncomment this block to use hardware SPI
#define OLED_DC     6
#define OLED_CS     7
#define OLED_RESET  8
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
*/

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ 
	B00000000, B11000000,
	B00000001, B11000000,
	B00000001, B11000000,
	B00000011, B11100000,
	B11110011, B11100000,
	B11111110, B11111000,
	B01111110, B11111111,
	B00110011, B10011111,
	B00011111, B11111100,
	B00001101, B01110000,
	B00011011, B10100000,
	B00111111, B11100000,
	B00111111, B11110000,
	B01111100, B11110000,
	B01110000, B01110000,
	B00000000, B00110000 
};

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {
	radio.begin();
	radio.setDataRate(RF24_250KBPS);
	radio.setPALevel(RF24_PA_MAX);
	radio.setPayloadSize(RF_PAYLOAD);
	radio.setChannel(RF_CHANNEL);
	radio.openReadingPipe(0, RF_ADDR);

	radio.startListening();

		// 如果需要，可以重设时间
		// Time t(2014, 5, 18, 21, 59, 55, Time::kSunday);
		// rtc.writeProtect(false);
		// rtc.time(t);
	rtc.halt(false);

	display.begin(SSD1306_SWITCHCAPVCC);
	display.display();
	delay(1000);
	display.clearDisplay();
}

char* dayOfWeek(const Time::Day day) {
	static char buff[64];
	switch (day) {
		case Time::kSunday: return strcpy(buff, "Sunday"), buff;
		case Time::kMonday: return strcpy(buff, "Monday"), buff;
		case Time::kTuesday: return strcpy(buff, "Tuesday"), buff;
		case Time::kWednesday: return strcpy(buff, "Wednesday"), buff;
		case Time::kThursday: return strcpy(buff, "Thursday"), buff;
		case Time::kFriday: return strcpy(buff, "Friday"), buff;
		case Time::kSaturday: return strcpy(buff, "Saturday"), buff;
	}
	return "(unknown day)";
}

char* now() {
	// Get the current time and date from the chip.
	Time t = rtc.time();
	// Format the time and date and insert into the temporary buffer.
	static char buf[128];
	sprintf(buf, "%02d:%02d:%02d\n%04d-%02d-%02d %s",
		t.hr, t.min, t.sec,
		t.yr, t.mon, t.date,
		dayOfWeek(t.day)
		);

	return buf;
}

char* readRadio() {
	static char buff[RF_PAYLOAD + 1];
	memset(buff, 0, RF_PAYLOAD + 1);
	if (radio.available()) {
		radio.read(buff, RF_PAYLOAD);
		return buff;
	}
	return 0;
}

void loop() {
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.println(now());
	display.display();

	display.setCursor(0,16);
	char* buf = 0;
	while(buf = readRadio()) {
		display.println(buf);
		display.display();
	}

	delay(100);
}
