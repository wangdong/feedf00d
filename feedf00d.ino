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
#include <string.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS1302.h>
#include <RF24.h>

const int RF_PAYLOAD_MAX = 32;
const int RF_PIN_CE  =  9;
const int RF_PIN_CSN = 10;
RF24 radio(RF_PIN_CE, RF_PIN_CSN);

const int kCePin   = 2;  // Chip Enable
const int kIoPin   = 3;  // Input/Output
const int kSclkPin = 4;  // Serial Clock
DS1302 rtc(kCePin, kIoPin, kSclkPin);

// If using software SPI (the default case):
#define OLED_MOSI  6
#define OLED_CLK   5
#define OLED_DC    8
#define OLED_CS    20
#define OLED_RESET 7
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup()   {
	Serial.begin(9600);

	radio.begin();
	radio.setDataRate(RF24_2MBPS);
	radio.setChannel(24);
	radio.openReadingPipe(0, 0x00FEEDF00D00ULL);
	radio.openReadingPipe(1, 0x00FEEDF00D01ULL);
	radio.openReadingPipe(2, 0x00FEEDF00D02ULL);
	radio.openReadingPipe(3, 0x00FEEDF00D03ULL);
	radio.openReadingPipe(4, 0x00FEEDF00D04ULL);
	radio.openReadingPipe(5, 0x00FEEDF00D05ULL);
	radio.setCRCLength(RF24_CRC_16);
	radio.enableDynamicPayloads();
	radio.setAutoAck(true);


	radio.startListening();

	printf_begin();
	radio.printDetails();


	// 如果需要，可以重设时间
	Time t(2014, 5, 18, 21, 59, 55, Time::kSunday);
	rtc.writeProtect(true);
	rtc.time(t);
	rtc.halt(false);

	display.begin(SSD1306_SWITCHCAPVCC);
	display.display();
	delay(200);
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
	Time t = rtc.time();
	static char buf[128];
	sprintf(buf, "%02d:%02d:%02d", t.hr, t.min, t.sec);
	// sprintf(buf, "%02d:%02d:%02d\n%04d-%02d-%02d %s",
	// 	t.hr, t.min, t.sec,
	// 	t.yr, t.mon, t.date,
	// 	dayOfWeek(t.day)
	// 	);
	return buf;
}

char* readRadio() {
	static char buff[RF_PAYLOAD_MAX + 1];
	memset(buff, 0, RF_PAYLOAD_MAX + 1);
	strcpy(buff, "NOSIGAL");
	if (radio.available()) {
		radio.read(buff, radio.getDynamicPayloadSize());
		char s[128] = {0};
		snprintf(s, 128, "%d:%s", radio.getDynamicPayloadSize(), buff);
		Serial.println(s);
		return buff;
	}
	return 0;
}

void loop() {
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.println(now());
	display.display();

	char* buf = 0;
	if (buf = readRadio()) {
		display.setCursor(0, 16);
		display.setTextSize(2);
		display.println(buf);
		display.display();
	}
}

int serial_putc( char c, FILE * )
{
  Serial.write( c );

  return c;
}

void printf_begin(void)
{
  fdevopen( &serial_putc, 0 );
}
