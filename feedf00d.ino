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
	radio.openReadingPipe(0, (uint64_t)*"L0N00");
	radio.openReadingPipe(1, (uint64_t)*"L0N01");
	radio.openReadingPipe(2, (uint64_t)*"L0N02");
	radio.openReadingPipe(3, (uint64_t)*"L0N03");
	radio.openReadingPipe(4, (uint64_t)*"L0N04");
	radio.openReadingPipe(5, (uint64_t)*"L0N05");
	radio.setCRCLength(RF24_CRC_16);
	radio.enableDynamicPayloads();
	radio.setAutoAck(true);
	radio.enableAckPayload();

	radio.startListening();

	printf_begin();
	radio.printDetails();


	// 如果需要，可以重设时间
	// rtc.time(Time(2014, 5, 21, 23, 15, 10, Time::kSunday));
	rtc.writeProtect(false);
	rtc.halt(false);

	display.begin(SSD1306_SWITCHCAPVCC);
	display.display();
	delay(500);
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

const char* keyframe[] = {">  ", ">> ", ">>>"};
#define countof(lst) (sizeof(lst)/sizeof(lst[0]))
void loop() {
	static class Anim {
		int index;
	public:
		Anim() : index(0) {
		}
		const char* nextFrame() {
			int current = index;
			index = ++index % countof(keyframe);
			return keyframe[current];
		}
	} anim;


	display.clearDisplay();

	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.println(now());

	display.setTextSize(1);
	display.setCursor(106,0);
	display.print(anim.nextFrame());

	char* buf = 0;
	uint8_t chanNum = -1;
	if (buf = readRadio(&chanNum)) {
		display.setCursor(0, 16);
		display.setTextSize(2);
		display.println(buf);
		display.setCursor(0, 56);
		display.setTextSize(1);
		display.print("CH");
		display.println(chanNum);
	} else {
		display.setCursor(0, 56);
		display.setTextSize(1);
		display.println("E:no signal");
	}
	display.display();
	delay(500);
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
