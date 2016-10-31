//#include <mcp_can.h>
#include <SPI.h>
#include "Arduino.h"
#include "HapcanDevice.h"

using namespace Onixarts::HomeAutomationCore;

Hapcan::HapcanDevice hapcanDevice;

void setup()
{
	Serial.begin(115200);
	Serial.println("Hapcanuino device starting...");

	hapcanDevice.Begin();
}

void loop()
{
	Hapcan::HapcanMessage* message = NULL;
	if (hapcanDevice.ReadRxBuffer(&message))
	{
		message->PrintToSerial();
	}

	//_delay_ms(50);
}