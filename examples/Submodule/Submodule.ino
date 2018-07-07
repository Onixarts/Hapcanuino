// Hapcanuino helps to implement Hapcan (Home Automation Project) compatible devices on Arduino board.
// 
// Code explanation: https://github.com/Onixarts/Hapcanuino/wiki/Submodule
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
#include <OnixartsIO.h>
#include <OnixartsTaskManager.h>
#include "SubModules\HapcanRelay\HapcanRelay.h"

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
																// Information type to be send by SendStatus function. Do not use value of 0 - it is reserved for StatusRequestType::SendAll

class MyLED7Device : public Hapcan::HapcanDeviceSubModuleHost<1>
{
	// submodules declaration
	Hapcan::SubModule::HapcanRelay::Module LED7Output;

public:
	MyLED7Device()
		: LED7Output(*this, 1, PIN7, 0x00)
	{
		// add SubModule to the host
		m_subModules[0] = &LED7Output;
	}
};

// and now, instantiate custom MyLED7Device class
MyLED7Device hapcanDevice;


void setup()
{
	Serial.begin(115200);
	Serial.println("Hapcanuino device starting...");

	hapcanDevice.Begin();
}

void loop()
{
	hapcanDevice.Update();
}
