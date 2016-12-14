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

// Callback function to be called, when received status request message
void OnStatusRequest(byte requestType, bool isAnswer);

// Information type to be send by OnStatusRequest function. Do not use value of 0 - it is reserved for SendAll
namespace StatusRequestType
{
	const byte LED7Info = 0x1;
}

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

	//set callback function for status request message
	hapcanDevice.SetStatusRequestDelegate(OnStatusRequest);

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
		// send status frame after change of LED7 to notify other Hapcan modules. 
		// Notice second (isAnswer) parameter is set to false, because we call it directly after status change
		OnStatusRequest(StatusRequestType::LED7Info, false);
	}
}

//Callback function is called automaticaly after status request message received.
// You can also call this function when status is changed to notify other Hapcan modules. See ExecuteInstruction function.
void OnStatusRequest(byte requestType, bool isAnswer)
{
	// check if we should send informations about all the functions in module
	bool sendAll = requestType == Hapcan::Message::System::StatusRequestType::SendAll;

	// if we need send all info or just LED7Info status
	if (sendAll || requestType == StatusRequestType::LED7Info)
	{
		// send message status of frame type 0x333 (custom). 
		// Use isAnswer variable here, because it will be set to true when it is a response for StatusRequest message (0x109)
		Hapcan::HapcanMessage statusMessage(0x333, isAnswer);
		statusMessage.m_data[2] = 7;	// set up byte 3 as 7
		statusMessage.m_data[3] = digitalRead(PIN7) == LOW ? 0x00 : 0x01; // set byte 4, 1 = LED ON, 0 = LED OFF
		hapcanDevice.Send(statusMessage);
	}

	// Add another status messages here...
	// if (sendAll || requestType == StatusRequestType::Your_another_defined_status)
	//{
	//}
}