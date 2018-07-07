// Hapcanuino helps to implement Hapcan (Home Automation Project) compatible devices on Arduino board.
// 
// Code explanation: https://github.com/Onixarts/Hapcanuino/wiki/Indirect-control
// Github: https://github.com/Onixarts/Hapcanuino
// Author's site: http://onixarts.pl
// Contact: software@onixarts.pl
// 
// Copyright (C) 2016-2018  Bartosz Rosa (onixarts.pl)
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

using namespace Onixarts::HomeAutomationCore;
Hapcan::HapcanDevice hapcanDevice;

// Configure Your Hapcan device here
const byte Hapcan::Config::MCP::InterruptPin = 2;				// CAN module interrupt is connected to this pin (see https://www.arduino.cc/en/Reference/AttachInterrupt)
const byte Hapcan::Config::MCP::CSPin = 10;						// SPI CS pin
const byte Hapcan::Config::MCP::OscillatorFrequency = MCP_8MHZ;	// MCP oscillator frequency on MCP CAN module (or MCP_16MHz)

const byte Hapcan::Config::Hardware::DeviceId1 = 0x12;			// unique device identifier 1, change it
const byte Hapcan::Config::Hardware::DeviceId2 = 0x34;			// unique device identifier 2, change it

const byte Hapcan::Config::Node::SerialNumber0 = 9;				// ID0 serial number MSB
const byte Hapcan::Config::Node::SerialNumber1 = 9;
const byte Hapcan::Config::Node::SerialNumber2 = 32;			// this is also a default node
const byte Hapcan::Config::Node::SerialNumber3 = 9;				// this is also a default group

const byte Hapcan::Config::Firmware::ApplicationType = 51;		// application (hardware) type (such as button, relay, dimmer) 1-10 Hapcan modules, 102 - ethernet, 51 - Hapcanuino example device
const byte Hapcan::Config::Firmware::ApplicationVersion = 0;	// application (hardware) version, change it with device hardware changes
const byte Hapcan::Config::Firmware::FirmwareVersion = 1;		// firmware version
const int  Hapcan::Config::Firmware::FirmwareRevision = 0;		// firmware revision
																// Configuration end

// Callback function to be called, when control message is received (or received message satisfy box criteria)
void ExecuteInstruction(Hapcan::InstructionStruct& exec, Hapcan::HapcanMessage& message);

// Callback function for status request handling
void SendStatus(byte requestType, bool isAnswer);

// Information type to be send by SendStatus function. Do not use value of 0 - it is reserved for StatusRequestType::SendAll
namespace StatusRequestType
{
	const byte LED7Status = 0x07;
	// const byte OtherDeviceStatus = 0x10;
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Hapcanuino device starting...");

	hapcanDevice.Begin();
	
	//set callback function to be called, when received message match box criteria or direct control message is received
	hapcanDevice.SetExecuteInstructionDelegate(ExecuteInstruction);

	// set callback function to be called when Hapcanuino receives status request message or module change its state
	hapcanDevice.SetStatusRequestDelegate(SendStatus);

	// demo example, set pin7 as output
	pinMode(PIN7, OUTPUT);
}

void loop()
{
	hapcanDevice.Update();
}

// Callback function is called when HAPCAN message match box criteria or direct control message is received
// @exec - instruction defined in box to be called when message is match criteria
// @message - hapcan message received from CAN
void ExecuteInstruction(Hapcan::InstructionStruct& exec, Hapcan::HapcanMessage& message)
{
	int initialState = digitalRead(PIN7);

	switch (exec.Instruction())
	{
	case 0: // turn LED OFF
		digitalWrite(PIN7, LOW);
		break;
	case 1: // turn LED ON
		digitalWrite(PIN7, HIGH);
		break;
	case 2: // toggle LED
		digitalWrite(PIN7, digitalRead(PIN7) == LOW);
		break;
		//case 3: // put other instructions here; break;
	}

	// check, if LED changes its state and send message to Hapcan bus
	if (initialState != digitalRead(PIN7))
	{
		// send status frame after change of LED7 to notify other Hapcan modules. 
		// Notice second (isAnswer) parameter is set to false, because we call it directly after status change, so it is not an answer for status request
		SendStatus(StatusRequestType::LED7Status, false);
	}
}

//Callback function is called automaticaly after status request message received.
// You can also call this function when status is changed to notify other Hapcan modules. See ExecuteInstruction function.
// @requestType - You can use this parameter to pass information about which device function status should be send. 
// You can use Hapcan::Config::Message::System::StatusRequestType::SendAll value (0) to send all values, and other custom device dependent value.
// @ isAnswer - when this function is called by Hapcanuino on status request message the true value is passed to this function. If You call this function on
// device state change, pass false, because other Hapcan devices may not process this message as it is marked as an answer for request.
void SendStatus(byte requestType, bool isAnswer)
{
	// check if we should send informations about all the functions in module
	bool sendAll = requestType == Hapcan::Message::System::StatusRequestType::SendAll;

	// if we need send all info or just LED7Info status
	if (sendAll || requestType == StatusRequestType::LED7Status)
	{
		// Prepare a standard Hapcan's Relay Message (0x302)
		// Use isAnswer variable here, because it will be set to true when it is a response for StatusRequest message (0x109)
		Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::RelayMessage, isAnswer);
		statusMessage.m_data[2] = 1;	// set up byte 3 as channel 1 for example
		statusMessage.m_data[3] = digitalRead(PIN7) == LOW ? 0x00 : 0xFF; // set byte 4 (status, 0x00 = LED OFF, 0xFF = LED ON
		hapcanDevice.Send(statusMessage);
	}

	//if (sendAll || requestType == StatusRequestType::OtherDeviceStatus)
	//{
		//... prepare and send OtherDeviceStatus message here
	//}
}