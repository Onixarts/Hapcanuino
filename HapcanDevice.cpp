#include "Arduino.h"
#include "HapcanDevice.h"
#include <avr/wdt.h>
#include <EEPROM.h>

using namespace Onixarts::HomeAutomationCore::Hapcan;

HapcanMessage::HapcanMessage()
: m_id(0)
{
	// reset all data fields
	memset(m_data, 0xFF, 8);
}

void HapcanMessage::PrintToSerial()
{
#ifdef OA_DEBUG
	//Serial.print("Raw Frame type: ");
	//Serial.println(m_id, HEX);
	Serial.print("Frame: 0x");

	unsigned int temp1 = GetFrameType();
	if (temp1 < 0x100)
		Serial.print("0");
	Serial.print(temp1, HEX);
	Serial.print(IsAnswer() ? " A" : "  ");

	Serial.print("\tNode (");
	Serial.print(GetNode());
	Serial.print(",");
	Serial.print(GetGroup());
	Serial.print(")");

	Serial.print("\t\tdata: ");
	for (int i = 0; i<8; i++)                // Print each byte of the data
	{
		if (m_data[i] < 0x10)                     // If data byte is less than 0x10, add a leading zero
		{
			Serial.print("0");
		}
		Serial.print(m_data[i], HEX);
		Serial.print(" ");
	}
	Serial.println();
#endif
}

HapcanDevice* pHapcanDevice = NULL;

HapcanDevice::HapcanDevice()
	:CAN(Config::MCP::CSPin)
	, m_RxBufferIndex(0)
	, m_RxBufferReadIndex(0)
	, m_rxBufferOverflowCount(0)
	, m_node(Hapcan::Config::Node::SerialNumber2)
	, m_group(Hapcan::Config::Node::SerialNumber3)
	, m_receiveAnswerMessages(false)
	, m_isInProgrammingMode(false)
	, m_isInitialized(false)
	, m_memoryAddress(0)
	, m_memoryCommand(Programming::Command::Undefined)
	, m_messageAcceptedDelegate(NULL)
{
	pHapcanDevice = this;
}

// Initiate Hapcan Device. Call this method in Arduino setup() function
void HapcanDevice::Begin()
{
	//Hapcan bus speed is 125Kbps
	CAN.begin(MCP_ANY, CAN_125KBPS, Config::MCP::OscillatorFrequency);
	CAN.setMode(MCP_NORMAL);

	pinMode(Config::MCP::InterruptPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(Config::MCP::InterruptPin), OnCanReceivedDispatcher, FALLING);

	ReadEEPROMConfig();

	m_isInitialized = true;
}

void HapcanDevice::ReadEEPROMConfig()
{
	// node reading
	m_node = EEPROM[0x26];

	// save initial node value to EEPROM if not set
	if (m_node == 0xFF)
	{
		EEPROM[0x26] = Config::Node::SerialNumber2;
		m_node = Config::Node::SerialNumber2;
	}

	m_group = EEPROM[0x27];

	// save initial group value to EEPROM if not set
	if (m_group == 0xFF)
	{
		EEPROM[0x27] = Config::Node::SerialNumber3;
		m_node = Config::Node::SerialNumber3;
	}

	// read description
	for (byte i = 0; i < 16; i++)
	{
		m_description[i] = EEPROM[0x30 + i];
		if (m_description[i] == 0xFF)
			m_description[i] = 0;
	}
}

// Add message to RX FIFO buffer, with overflow check
void HapcanDevice::AddMessageToRxBuffer(HapcanMessage& message)
{
	m_RxBuffer[m_RxBufferIndex] = message;
	m_RxBufferIndex++;
	if (m_RxBufferIndex >= Config::RxFifoQueueSize)
	m_RxBufferIndex = 0;

	// rx buffer overflow check
	if (m_RxBufferIndex == m_RxBufferReadIndex)
	{
		m_rxBufferOverflowCount++;
		OA_LOG_LINE("RX Buffer overflow. Count = ");
		OA_LOG(m_rxBufferOverflowCount);
	}
}

// Read one message from RX FIFO buffer.
// Returns true if there is any message to read
bool HapcanDevice::ReadRxBuffer(HapcanMessage ** message)
{
	// no messages waiting
	if (m_RxBufferReadIndex == m_RxBufferIndex)
		return false;

	*message = &m_RxBuffer[m_RxBufferReadIndex];

	m_RxBufferReadIndex++;
	if (m_RxBufferReadIndex >= Config::RxFifoQueueSize)
		m_RxBufferReadIndex = 0;

	return true;
}


// static CAN MCP interupt callback function
void HapcanDevice::OnCanReceivedDispatcher()
{
	if (pHapcanDevice != NULL)
		pHapcanDevice->OnCanReceived();
}

// Method is called on CAN MCP Interrupt. It reads message and put it into RX FIFO buffer
void HapcanDevice::OnCanReceived()
{
	// don't process any message until device is full initialized
	if (!m_isInitialized)
		return;

	byte len = 0;
	byte rxBuffer[8];
	long unsigned int rxId;
	byte ext;
	CAN.readMsgBuf(&rxId, &ext, &len, rxBuffer);
	HapcanMessage hapcanMessage;
	hapcanMessage.Parse(rxId, rxBuffer);

	if (!m_receiveAnswerMessages && hapcanMessage.IsAnswer())				// ignore answer messages
		return;

	AddMessageToRxBuffer(hapcanMessage);
}

// Perform rx buffer reading and processing. Call this methon in loop() function. 
void HapcanDevice::Update()
{
	ProcessRxBuffer();
}

// Checks if there is any new message to process and perform processing in this case
// Returns true if message processed
bool HapcanDevice::ProcessRxBuffer()
{
	Hapcan::HapcanMessage* message = NULL;
	if (ReadRxBuffer(&message))
	{
		message->PrintToSerial();

		byte frameTypeCategory = message->GetFrameTypeCategory();

		if (m_isInProgrammingMode)
		{
			ProcessProgrammingMessage(message);
			return true;
		}

		if (frameTypeCategory < Hapcan::Message::NormalMessageCategory)
			ProcessSystemMessage(message);
		else
			ProcessNormalMessage(message);

		return true;
	}
	return false;
}

// Checks if message is for nodes in current group or for all groups
bool HapcanDevice::MatchGroup(HapcanMessage* message)
{
	if (message->m_data[2] == 0 && (message->m_data[3] == m_group || message->m_data[3] == 0x00))
		return true;
	return false;
}

// Checks if message is for this node
bool HapcanDevice::MatchNode(HapcanMessage* message)
{
	if (message->m_data[2] == m_node && message->m_data[3] == m_group)
		return true;
	return false;
}

void HapcanDevice::ProcessProgrammingMessage(HapcanMessage* message)
{
	unsigned int frameType = message->GetFrameType();
	switch (frameType)
	{
	case Message::System::AddressFrame:
		AddressFrameAction(message);
		break;
	case Message::System::DataFrame:
		DataFrameAction(message);
		break;
	case Message::System::ExitAllFromBootloaderProgrammingMode:
		ProgrammingModeAction(frameType);
		break;
	case Message::System::ExitOneNodeFromBootloaderProgrammingMode:
		if (message->GetNode() == m_node && message->GetGroup() == m_group)
			ProgrammingModeAction(frameType);
		break;
	}
}

// Process normal message type. Checks box enable bits and test HAPCAN message conditions. 
bool HapcanDevice::ProcessNormalMessage(HapcanMessage* message)
{
	//unsigned long aaa1 = millis();
	unsigned int boxConfigAddress = CoreConfig::EEPROM::BoxConfigAddress;
	for (byte i = 0; i < CoreConfig::BoxCount/8; i++)
	{
		byte boxEnableFlags = EEPROM[CoreConfig::EEPROM::BoxEnableAddress + i];
		for (byte boxBit = 0; boxBit < 8; boxBit++)
		{
			if (boxEnableFlags & 0x01)
			{
				BoxConfigStruct boxConfig;
				//Serial.println(CoreConfig::EEPROM::BoxConfigAddress + sizeof(BoxConfigStruct)*boxBit + i * 8 * sizeof(BoxConfigStruct), HEX);
				EEPROM.get(CoreConfig::EEPROM::BoxConfigAddress + sizeof(BoxConfigStruct)*boxBit + i*8*sizeof(BoxConfigStruct), boxConfig);
				if (boxConfig.Accept(message))
				{
					OA_LOG("Accepted");
					if (m_messageAcceptedDelegate != NULL)
						m_messageAcceptedDelegate(message, boxConfig.data[15], boxConfig.data[16], boxConfig.data[17], boxConfig.data[18]);


				}
			}
			boxEnableFlags = boxEnableFlags >> 1;
		}
	}
	return false;
	//unsigned long aaa2 = millis();
	//Serial.println(aaa1);
	//Serial.print("-");
	//Serial.println(aaa2);
}

// Process system massage type
bool HapcanDevice::ProcessSystemMessage(HapcanMessage* message)
{
	unsigned int frameType = message->GetFrameType();
	switch(frameType)
	{
	case Message::System::EnterProgrammingMode:
		if (MatchNode(message))
			EnterProgrammingModeAction(frameType);
		break;
	case Message::System::RebootRequestToGroup:
		if (MatchGroup(message))
			RebootAction();
		break;
	case Message::System::RebootRequestToNode:
		if (MatchNode(message))
			RebootAction();
		break;
	case Message::System::HardwareTypeRequestToGroup: 
		if(MatchGroup(message))
			CanNodeIdAction(frameType);
		break;
	case Message::System::HardwareTypeRequestToNode:
		if (MatchNode(message))
			CanNodeIdAction(frameType);
		break;
	case Message::System::FirmwareTypeRequestToGroup:
		if (MatchGroup(message))
			CanFirmwareIdAction(frameType);
		break;
	case Message::System::FirmwareTypeRequestToNode:
		if (MatchNode(message))
			CanFirmwareIdAction(frameType);
		break;
	case Message::System::SetDefaultNodeAndGroupRequestToNode:
		if (MatchNode(message))
			SetDefaultNodeAndGroupAction(frameType);
		break;
	case Message::System::StatusRequestToGroup:
		if (MatchGroup(message))
			StatusRequestAction(message);
		break;
	case Message::System::StatusRequestToNode:
		if (MatchNode(message))
			StatusRequestAction(message);
		break;
	case Message::System::ControlMessage:
		if (MatchNode(message))
			ControlAction(message);
		break;

	case Message::System::SupplyVoltageRequestToGroup:
		if (MatchGroup(message))
			SupplyVoltageAction(frameType);
		break;
	case Message::System::SupplyVoltageRequestToNode:
		if (MatchNode(message))
			SupplyVoltageAction(frameType);
		break;

	case Message::System::DescriptionRequestToGroup:
		if (MatchGroup(message))
			NodeDescriptionAction(frameType);
		break;
	case Message::System::DescriptionRequestToNode:
		if (MatchNode(message))
			NodeDescriptionAction(frameType);
		break;
	case Message::System::DeviceIDRequestToGroup:
		if (MatchGroup(message))
			DeviceIDAction(frameType);
		break;

	case Message::System::DeviceIDRequestToNode:
		if (MatchNode(message))
			DeviceIDAction(frameType);
		break;
	//case Message::System::UptimeRequestToGroup:
	//	if (MatchGroup(message))
	//		UptimeAction(frameType);
	//	break;
	//case Message::System::UptimeRequestToNode:
	//	if (MatchNode(message))
	//		UptimeAction(frameType);
	//	break;

		//const unsigned int HealthCheckRequestToGroup = 0x114;
		//const unsigned int HealthCheckRequestToNode = 0x115;
	default: 
		return false;
	}

	return true;

}

// Returns RX FIFO buffer overflow count
unsigned long HapcanDevice::GetRxBufferOverflowCount()
{
	return m_rxBufferOverflowCount;
}

// Send HapcanMessage to the CAN BUS
void HapcanDevice::Send(HapcanMessage& message)
{
	message.PrintToSerial();
	CAN.sendMsgBuf(message.m_id, 1, 8, message.m_data);
}

//--------------------------------------------------------------------------------------------------------------------
//-- BOOTLOADER ACTIONS ----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------

// Enter programming mode
void HapcanDevice::EnterProgrammingModeAction(unsigned int frameType)
{
	m_isInProgrammingMode = true;
	
	HapcanMessage message;
	message.BuildIdPart(frameType, true, m_node, m_group);
	message.m_data[2] = Config::BootLoader::BootloaderVersion;
	message.m_data[3] = Config::BootLoader::BootloaderRevision;

	OA_LOG("> Entering programming mode");
	Send(message);
}

// Address frame handling in programming mode
void HapcanDevice::AddressFrameAction(HapcanMessage* inputMessage)
{
	//Currently only EEPROM programming is supported. Hapcan EEPROM is on 0xF00000 - 0xF003FF (1kB)
	if (inputMessage->m_data[0] != 0xF0)
	{
		ErrorFrameAction(inputMessage);
		return;
	}

	m_memoryAddress = ((int) inputMessage->m_data[1] << 8) + inputMessage->m_data[2];
	m_memoryCommand = inputMessage->m_data[5];

	if (m_memoryAddress > EEPROM.length())
	{
		ErrorFrameAction(inputMessage);
		return;
	}

	inputMessage->SetAnswer();

	OA_LOG("> Address frame");
	Send(*inputMessage);
}

// Data frame handling in programming mode
void HapcanDevice::DataFrameAction(HapcanMessage* inputMessage)
{
	switch (m_memoryCommand)
	{
	case Programming::Command::Undefined:
		ErrorFrameAction(inputMessage);
		return;
	case Programming::Command::Read:
		for (byte i = 0; i < 8; i++)
			inputMessage->m_data[i] = EEPROM[m_memoryAddress+i];
		break;
	case Programming::Command::Write:
		for (byte i = 0; i < 8; i++)
		{
			EEPROM[m_memoryAddress + i] = inputMessage->m_data[i];
			inputMessage->m_data[i] = EEPROM[m_memoryAddress + i]; // read EEPROM and send it back, it should be the same
		}
		break;
	}

	inputMessage->SetAnswer();

	OA_LOG("> Data frame");
	Send(*inputMessage);
}

// Send error frame
void HapcanDevice::ErrorFrameAction(HapcanMessage* inputMessage)
{
	HapcanMessage message;
	message.BuildIdPart(Message::System::ErrorFrame, true, m_node, m_group);
	message.m_data[2] = Config::BootLoader::BootloaderVersion;
	message.m_data[3] = Config::BootLoader::BootloaderRevision;

	OA_LOG("> Error Frame");
	Send(message);
}

// Handle programming mode 
void HapcanDevice::ProgrammingModeAction(unsigned int frameType)
{
	switch (frameType)
	{
	case Message::System::ExitAllFromBootloaderProgrammingMode:
	case Message::System::ExitOneNodeFromBootloaderProgrammingMode:
		OA_LOG("> Exiting programming mode");
		if (m_isInProgrammingMode)
			RebootAction();
		break;
	}

}

// Reboots the device. No Message is sent to CAN BUS.
void HapcanDevice::RebootAction()
{
	OA_LOG("> Rebooting...");
	wdt_enable(WDTO_15MS);
	_delay_ms(20);
}

// Send Can Node ID
void HapcanDevice::CanNodeIdAction(unsigned int frameType)
{
	HapcanMessage message;
	message.BuildIdPart(frameType, true, m_node, m_group);
	message.m_data[0] = Config::Hardware::HardwareType1;
	message.m_data[1] = Config::Hardware::HardwareType2;
	message.m_data[2] = Config::Hardware::HardwareVersion;
	
	message.m_data[4] = Config::Node::SerialNumber0;
	message.m_data[5] = Config::Node::SerialNumber1;
	message.m_data[6] = Config::Node::SerialNumber2;
	message.m_data[7] = Config::Node::SerialNumber3;
	
	OA_LOG("> CanNodeId");
	Send(message);
}

// Send Can Firmware ID or error frame if no firmware
void HapcanDevice::CanFirmwareIdAction(unsigned int frameType)
{
	//TODO: test if firmware is OK, return error frame then
	HapcanMessage message;
	message.BuildIdPart(frameType, true, m_node, m_group);
	message.m_data[0] = Config::Hardware::HardwareType1;
	message.m_data[1] = Config::Hardware::HardwareType2;
	message.m_data[2] = Config::Hardware::HardwareVersion;
	message.m_data[3] = Config::Firmware::ApplicationType;
	message.m_data[4] = Config::Firmware::ApplicationVersion;
	message.m_data[5] = Config::Firmware::FirmwareVersion;
	message.m_data[6] = Config::BootLoader::BootloaderVersion;
	message.m_data[7] = Config::BootLoader::BootloaderRevision;
	
	OA_LOG("> CanFirmwareId");
	Send(message);
}

// Send supply voltage information (currently not supported, returns 0V)
void HapcanDevice::SupplyVoltageAction(unsigned int frameType)
{
	HapcanMessage message;
	message.BuildIdPart(frameType, true, m_node, m_group);
	message.m_data[0] = 0;
	message.m_data[1] = 0;
	message.m_data[2] = 0;
	message.m_data[3] = 0;

	OA_LOG("> SupplyVoltage");
	Send(message);
}

void HapcanDevice::NodeDescriptionAction(unsigned int frameType)
{
	HapcanMessage message;
	message.BuildIdPart(frameType, true, m_node, m_group);
	for(byte i = 0; i < 8; i++)
		message.m_data[i] = m_description[i];

	OA_LOG("> Description 1");
	Send(message);

	for (byte i = 0; i < 8; i++)
		message.m_data[i] = m_description[8+i];

	OA_LOG("> Description 2");
	Send(message);
}

// Reset node and group to default values
void HapcanDevice::SetDefaultNodeAndGroupAction(unsigned int frameType)
{
	EEPROM[0x26] = Config::Node::SerialNumber2;
	EEPROM[0x27] = Config::Node::SerialNumber3;

	m_node = Config::Node::SerialNumber2;
	m_group = Config::Node::SerialNumber3;

	HapcanMessage message;
	message.BuildIdPart(frameType, true, m_node, m_group);

	OA_LOG("> SetDefaultNodeAndGroup");
	Send(message);
}

// Default implementation for status Request. Override this method in derived class 
void HapcanDevice::StatusRequestAction(HapcanMessage* message)
{
	HapcanMessage messageOut;
	messageOut.BuildIdPart(message->GetFrameType(), true, m_node, m_group);

	OA_LOG("> StatusRequest");
	Send(messageOut);
}

// Default implementation for control module. Override this method in derived class 
void HapcanDevice::ControlAction(HapcanMessage* message)
{
	HapcanMessage messageOut;
	messageOut.BuildIdPart(message->GetFrameType(), true, m_node, m_group);

	OA_LOG("> Control");
	Send(messageOut);
}

// Send device (chip) ID 
void HapcanDevice::DeviceIDAction(unsigned int frameType)
{
	HapcanMessage message;
	message.BuildIdPart(frameType, true, m_node, m_group);
	message.m_data[0] = Config::Hardware::DeviceId1;
	message.m_data[1] = Config::Hardware::DeviceId2;

	OA_LOG("> DeviceID");
	Send(message);
}