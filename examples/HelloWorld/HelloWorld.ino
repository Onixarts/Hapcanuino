// Hapcanuino helps to implement Hapcan (Home Automation Project) compatible devices on Arduino board.
// 
// Code explanation: https://github.com/Onixarts/Hapcanuino/wiki/HelloWorld
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

// Callback function to be called, when received message match box criteria or direct control message is received
void ExecuteInstruction(byte instruction, byte param1, byte param2, byte param3, Hapcan::HapcanMessage& message);

void setup()
{
	Serial.begin(115200);
	Serial.println("Hapcanuino device starting...");

	// initializing Hapcanuino device
	hapcanDevice.Begin();

	// uncomment this to receive answer messages. Can cause RX buffer overflow and frame loss
	//hapcanDevice.ReceiveAnswerMessages(true);

	//set callback function to be called, when received message match box criteria or direct control message is received
	hapcanDevice.SetExecuteInstructionDelegate(ExecuteInstruction);

	// demo example, set pin7 as output
	pinMode(PIN7, OUTPUT);
}

void loop()
{
	// call Update in loop, to process incomming CAN messages. Should be called as frequent as possible
	hapcanDevice.Update();

	// TODO: place your loop code here. This code should not block loop function for a long time
}

// Callback function is called when HAPCAN message match box criteria or direct control message is received
// @instruction - instruction defined in box to be called when message is match criteria
// @param1-3 - parameters defined in box for instruction
// @message - hapcan message received from CAN
void ExecuteInstruction(byte instruction, byte param1, byte param2, byte param3, Hapcan::HapcanMessage& message)
{
	bool ledStateChanged = false;

	switch (instruction)
	{
	case 1: // turn LED ON
		digitalWrite(PIN7, HIGH);
		ledStateChanged = true;
		break;
	case 2: // turn LED OFF
		digitalWrite(PIN7, LOW);
		ledStateChanged = true;
		break;
	case 3: // toggle LED
		digitalWrite(PIN7, digitalRead(PIN7) == LOW);
		ledStateChanged = true;
		break;
	//case 4: // put other instructions here; break;
	}

	// check, if LED change instruction was executed and send message to Hapcan
	if (ledStateChanged)
	{
		// send message confirmed status change of frame type 0x333 (custom)
		Hapcan::HapcanMessage statusMessage(0x333, false);
		statusMessage.m_data[2] = 7;	// set up byte 3 as 7
		statusMessage.m_data[3] = digitalRead(PIN7) == LOW ? 0x00 : 0x01; // set byte 4, 1 = LED ON, 0 = LED OFF
		hapcanDevice.Send(statusMessage);
	}
}