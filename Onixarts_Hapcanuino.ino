//#include <SPI.h>
#include <U8glib.h>
#include "Arduino.h"
#include "HapcanDevice.h"


using namespace Onixarts::HomeAutomationCore;

Hapcan::HapcanDevice hapcanDevice;

void DoInstruction(Hapcan::HapcanMessage* message, byte instruction, byte param1, byte param2, byte param3);
void draw(void);// example

//example
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);	// I2C / TWI 
const byte textSize = 10;
char OLEDText[textSize];
bool OLEDNeedRefresh = true;

void setup()
{
	Serial.begin(115200);
	Serial.println("Hapcanuino device starting...");

	hapcanDevice.Begin();
	hapcanDevice.ReceiveAnswerMessages(true);

	//example
	hapcanDevice.OnMessageAcceptedEvent(DoInstruction);
	pinMode(PIN7, OUTPUT);

	memset(OLEDText, 0, textSize);

	// assign default color value
	if (u8g.getMode() == U8G_MODE_R3G3B2) {
		u8g.setColorIndex(255);     // white
	}
	else if (u8g.getMode() == U8G_MODE_GRAY2BIT) {
		u8g.setColorIndex(3);         // max intensity
	}
	else if (u8g.getMode() == U8G_MODE_BW) {
		u8g.setColorIndex(1);         // pixel on
	}
	else if (u8g.getMode() == U8G_MODE_HICOLOR) {
		u8g.setHiColorByRGB(255, 255, 255);
	}
}

void loop()
{
	hapcanDevice.Update();


	if (OLEDNeedRefresh)
	{
		OLEDNeedRefresh = false;
		// picture loop
		u8g.firstPage();
		do {
			draw();
		} while (u8g.nextPage());
	}
}

void DoInstruction(Hapcan::HapcanMessage* message, byte instruction, byte param1, byte param2, byte param3)
{
	switch (instruction)
	{
	case 1: digitalWrite(PIN7, digitalRead(PIN7) == LOW);
		break;
	case 2: 
		memset(OLEDText, 0, textSize);
		OLEDText[0] = (message->m_data[5] >> 4 ) + 48;
		OLEDText[1] = (message->m_data[5] & 0x0F) + 48;
		OLEDText[2] = ':';
		OLEDText[3] = (message->m_data[6] >> 4) + 48;
		OLEDText[4] = (message->m_data[6] & 0x0F) + 48;
		OLEDNeedRefresh = true;
		break;
	}
	
}

void draw(void)
{
	// graphic commands to redraw the complete screen should be placed here  
	u8g.setFont(u8g_font_unifont);
	//u8g.setFont(u8g_font_osb21);
	u8g.drawStr(25, 10, "Hapcanuino");
	u8g.setFont(u8g_font_fub20n);
	u8g.drawStr(30, 40, OLEDText);
}