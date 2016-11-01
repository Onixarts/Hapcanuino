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
	hapcanDevice.ReceiveAnswerMessages(true);
}

void loop()
{
	hapcanDevice.Update();
}