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
#include <DS1302.h>
#include <RF24.h>

#include <Max72xxPanel.h>

int pinCS = 8; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 2;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

const int RF_PAYLOAD_MAX = 32;
const int RF_PIN_CE  =  9;
const int RF_PIN_CSN = 10;
RF24 radio(RF_PIN_CE, RF_PIN_CSN);

const int kCePin   = 5;  // Chip Enable
const int kIoPin   = 6;  // Input/Output
const int kSclkPin = 7;  // Serial Clock
DS1302 rtc(kCePin, kIoPin, kSclkPin);


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
	// rtc.time(Time(2014, 5, 25, 1, 58, 30, Time::kSunday));
	rtc.writeProtect(false);
	rtc.halt(false);

	matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
}

char* now() {
	Time t = rtc.time();
	static char buf[128];
	sprintf(buf, "%02d:%02d:%02d %02d-%02d",
		t.hr, t.min, t.sec,
	 	t.mon, t.date
	 	);
	return buf;
}

char* readRadio(uint8_t* pipenum = NULL) {
	static char buff[RF_PAYLOAD_MAX + 1];
	memset(buff, 0, RF_PAYLOAD_MAX + 1);
	if (radio.available(pipenum)) {
		radio.read(buff, radio.getDynamicPayloadSize());
		char s[128] = {0};
		snprintf(s, 128, "%d:%s", radio.getDynamicPayloadSize(), buff);
		Serial.println(s);
		return buff;
	}
	return 0;
}

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels
int spd = 30;

void loop() {
	String tape = now();
	char* buf = 0;
	uint8_t chanNum = -1;
	if (buf = readRadio(&chanNum)) {
		tape += " ";
		tape += buf;
	} else {
		tape += " NOSIG";
	}
	for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer; i++ ) {

		matrix.fillScreen(LOW);

		int letter = i / width;
		int x = (matrix.width() - 1) - i % width;
		int y = (matrix.height() - 8) / 2; // center the text vertically

		while ( x + width - spacer >= 0 && letter >= 0 ) {
		  if ( letter < tape.length() ) {
		    matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
		  }

		  letter--;
		  x -= width;
		}

		matrix.write(); // Send bitmap to display
		delay(spd);
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
