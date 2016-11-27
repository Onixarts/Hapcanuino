// Hapcanuino helps to implement Hapcan (Home Automation Project) compatible device on Arduino board.
// Github: https://github.com/Onixarts/Hapcanuino
// Author's site: http://onixarts.pl
// Contact: software@onixarts.pl
// 
// Copyright (C) 2016  Bartosz Rosa (onixarts.pl)
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.

#include "Arduino.h"
#include "HapcanDevice.h"

// Hapcanuino uses namespaces 
using namespace Onixarts::HomeAutomationCore;

// HapcanDevice class declaration
Hapcan::HapcanDevice hapcanDevice;

// Callback function to be called, when received message match box criteria
void DoInstruction(Hapcan::HapcanMessage* message, byte instruction, byte param1, byte param2, byte param3);

void setup()
{
	Serial.begin(115200);
	Serial.println("Hapcanuino device starting...");

	// initializing Hapcanuino device
	hapcanDevice.Begin();

	// uncomment this to receive answer messages. Can cause RX buffer overflow and frame loss
	//hapcanDevice.ReceiveAnswerMessages(true);

	//set callback function to be called, when received message match box criteria
	hapcanDevice.OnMessageAcceptedEvent(DoInstruction);

	// demo example, set pin7 as output
	pinMode(PIN7, OUTPUT);
}

void loop()
{
	// call Update in loop, to process incomming CAN messages. Should be called as frequent as possible
	hapcanDevice.Update();

	// TODO: place your loop code here. This code should not block loop function for a long time
}

// Callback function is called when HAPCAN message match criteria
// @message - hapcan message received from CAN
// @instruction - instruction defined in box to be called when message is match criteria
// @param1-3 - parameters defined in box for instruction
void DoInstruction(Hapcan::HapcanMessage* message, byte instruction, byte param1, byte param2, byte param3)
{
	switch (instruction)
	{
	case 1: digitalWrite(PIN7, digitalRead(PIN7) == LOW);
		break;
		// TODO: place other instructions here
	}
}