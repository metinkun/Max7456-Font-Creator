#include <SPI.h>
#include <MAX7456.h>


#define MAX7456_PAL        1 
#define MAX7456_ASCII      2

const byte osdChipSelect = 8;
const byte masterOutSlaveIn = MOSI;
const byte masterInSlaveOut = MISO;
const byte slaveClock = SCK;
const byte osdReset = 9;

MAX7456 OSD(osdChipSelect);


byte charbuf[54]; // for NVM_read

#include <avr/pgmspace.h>
#include "MAX7456_Font.h"




				  //////////////////////////////////////////////////////////////
void setup()
{
	delay(1000);
	Serial.begin(115200);
	Serial.flush();
	Serial.println("loading");
	hardReset();
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV2);      // Must be less than 10MHz.
											  // Initialize the MAX7456 OSD:
	OSD.begin();                                // Use NTSC with default area.
	OSD.setDefaultSystem(MAX7456_PAL);
	OSD.setSwitchingTime(5);                  // Set video chroma distortion 
											  //   to a minimum.
	OSD.display();                              // Activate the text display.
	OSD.setCharEncoding(MAX7456_ASCII);         // Use non-decoded access.
	OSD.noLineWrap();                           // Set wrap behaviour.
	OSD.noPageWrap();

	//reset_max7456();
	Serial.println("Ready for commands: D - download font");

	//display all 256 internal MAX7456 characters
	show_font();

	Serial.print("Embedded font is ");
	Serial.print(sizeof(fontdata));
	Serial.println("bytes long.");
	Serial.println("MAX7456>");
}

//////////////////////////////////////////////////////////////
void loop()
{
	if (Serial.available() > 0)
	{
		// read the incoming byte:
		byte incomingByte = Serial.read();
		switch (incomingByte) // wait for commands
		{
		case 'D': // download font
			transfer_fontdata();
			break;
		case 'r': // reset
			OSD.reset();
			break;
		case 'R': // read font 
				  //       read_fontdata();
			break;
		case 's': // show charset
			show_font();
			break;
		case '?': // read status
			Serial.print("STAT=");
			Serial.println(OSD.status());
			break;
		default:
			Serial.println("invalid command");
			break;
		}
		Serial.println("MAX7456>");
	}
}

void hardReset()
{
	pinMode(osdReset, OUTPUT);
	digitalWrite(osdReset, HIGH);
	delay(10);
	digitalWrite(osdReset, LOW);
	delay(10);
	digitalWrite(osdReset, HIGH);
}


//////////////////////////////////////////////////////////////
void show_font() //show all chars on 24 wide grid
{
	unsigned short x;

	OSD.clear();
	delay(2); // clearing display takes 20uS so wait some...

	while (OSD.notInVSync());
	for (byte b = 0; b < 13; b++)
	{
		for (byte a = 0; a < 20; a++)
		{
			OSD.setCursor(a, b); // col , row
			OSD.write(b*20 + a);                  // write symbol to display
		}
	}
}

void transfer_fontdata()
{
	if (sizeof(fontdata) != 16384) {
		Serial.println("ERROR: fontdata with invalid size, aborting!!!");
		return;
	}
	Serial.println("Downloading font to MAX7456 NVM, this may take a while...");
	for (int ch = 0; ch<256; ch++) {
		Serial.print((int)ch);
		if(OSD.createChar(ch, fontdata + (64 * ch)))
			Serial.println(" OK");
		else
		{
			ch--;
			Serial.println(" FAIL");
		}
		delay(30);
	}
	// force soft reset on Max7456
	Serial.println(" Reseting");
	hardReset();
	delay(1000);
	Serial.println(" OK");
	OSD.begin();                                // Use NTSC with default area.
	OSD.setDefaultSystem(MAX7456_PAL);
	OSD.setSwitchingTime(5);                  // Set video chroma distortion 
											  //   to a minimum.
	OSD.display();                              // Activate the text display.
	OSD.setCharEncoding(MAX7456_ASCII);         // Use non-decoded access.
	OSD.noLineWrap();                           // Set wrap behaviour.
	OSD.noPageWrap();


	show_font();
	Serial.println("");
	Serial.println("Done with font download");
}