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


//
// Pin Map
//
const int PIN_CE_DS1302     = 5;
const int PIN_IO_DS1302     = 6;
const int PIN_SCK_DS1302    = 7;
const int PIN_CSN_LEDMATRIX = 8;
const int PIN_CE_RF24       = 9;
const int PIN_CSN_RF24      = 10;

//
// LED Matrix
//
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

String lastMsg;

const int spacer 					= 1;
const int width 					= 5 + spacer; // The font width is 5 pixels
const int scroll_speed 				= 30;
const int countHorizontalDisplays 	= 2;
const int countVerticalDisplays   	= 1;
Max72xxPanel matrix(PIN_CSN_LEDMATRIX, countHorizontalDisplays, countVerticalDisplays);

//
// RF24
//
#include <RF24.h>
const int RF_PAYLOAD_MAX = 32;
RF24 radio(PIN_CE_RF24, PIN_CSN_RF24);

//
// RTC
//
#include <DS1302.h>
DS1302 rtc(PIN_CE_DS1302, PIN_IO_DS1302, PIN_SCK_DS1302);


void setup()   {
	Serial.begin(9600);

	//
	// RADIO
	// 
	radio.begin();
	radio.setDataRate(RF24_2MBPS);
	radio.setChannel(24);
	radio.openReadingPipe(0, 0x00FEEDF00D00ULL);
	radio.openReadingPipe(1, 0x00FEEDF00D11ULL);
	radio.openReadingPipe(2, 0x00FEEDF00D12ULL);
	radio.openReadingPipe(3, 0x00FEEDF00D13ULL);
	radio.openReadingPipe(4, 0x00FEEDF00D14ULL);
	radio.openReadingPipe(5, 0x00FEEDF00D15ULL);
	radio.setCRCLength(RF24_CRC_16);
	radio.enableDynamicPayloads();
	radio.setAutoAck(true);

	radio.startListening();

	printf_begin();
	radio.printDetails();

	//
	// RTC
	//
	// 如果需要，可以重设时间
	// rtc.time(Time(2014, 5, 25, 1, 58, 30, Time::kSunday));
	rtc.writeProtect(false);
	rtc.halt(false);

	//
	// LED Matrix
	//
	matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
}

char* now() {
	static char buf[32];
	Time t = rtc.time();
	sprintf(buf, "%02d:%02d", t.hr, t.min);
	return buf;
}

const char* readRadio(uint8_t* pipenum = NULL) {
	static char buff[RF_PAYLOAD_MAX + 1];
	memset(buff, 0, RF_PAYLOAD_MAX + 1);
	if (radio.available(pipenum)) {
		radio.read(buff, radio.getDynamicPayloadSize());
		return buff;
	}
	return 0;
}

void loop() {
	if (const char* buf = readRadio()) {
		lastMsg  = " todo ";
		lastMsg += buf;
	}
	
	String tape = now();
	tape += lastMsg;

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

		matrix.write();
		delay(scroll_speed);
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
