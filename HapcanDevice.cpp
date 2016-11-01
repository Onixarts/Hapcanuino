#include "Arduino.h"
#include "HapcanDevice.h"

// Send debug info to Serial port. Comment line below to disable Serial notifications and reduce code and SRAM usage
#define OA_DEBUG 1	

#ifdef OA_DEBUG
	#define OA_LOG(text) Serial.println(text)
#else
	#define OA_LOG(text) //empty
#endif	


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
	Serial.print("Frame: ");
	byte temp = GetFrameTypeCategory();
	if( temp < 0x10)
		Serial.print("0");
	Serial.print(temp, HEX);
	Serial.print(" ");
	temp = GetFrameType();
	if (temp < 0x10)
		Serial.print("0");
	Serial.print(temp, HEX);

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
	, m_group(Hapcan::Config::InitialGroup)
	, m_node(Hapcan::Config::InitialNode)
	, m_receiveAnswerMessages(false)
{
	pHapcanDevice = this;

}

// Initiate Hapcan Device. Call this method in Arduino setup() function
void HapcanDevice::Begin()
{
	//Hapcan bus speed is 125Kbps
	CAN.begin(CAN_125KBPS, Config::MCP::OscillatorFrequency);

	pinMode(Config::MCP::InterruptPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(Config::MCP::InterruptPin), OnCanReceivedDispatcher, FALLING);

	//ReadEEpromConfig()
	// odczyt node number i group z eepromu
	// firmware started
	//FIRMFLAG |= 1;      // set flag "firmware started and ready for interrupts"
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

	//// main firmware ready flag
	//if (!(FIRMFLAG & 1))         // main firmware is not ready yet
	//	return;
	//if (!(CANFULL & 1))
	//	return;

	//	return;
	//if (RXFRAME1 != 0x10) // accept only frames beginning with 0x10
	//	return;

	//switch (RXFRAME2) {
	//case 0x80:
	//	if (RXD2 != 0)                   // reject ask with GROUP_ID!=0
	//		return;
	//	if (RXD3 != 0 && RXD3 != TXGROUP)  // accept ask to this group or all groups
	//		return;
	//	// code
	//	break;
	//case 0x90:
	//	if (RXD2 != TXNODE || RXD3 != TXGROUP)   // accept ask to this module
	//		return;
	//	// code
	//	break;
	//case 0xA0:
	//	if (RXD2 != TXNODE || RXD3 != TXGROUP)   // acceptask to this module
	//		return;
	//	// code
	//	break;
	//default:
	//	return;
	//}



	byte len = 0;
	byte rxBuffer[8];
	CAN.readMsgBuf(&len, rxBuffer);              // Read data: len = data length, buf = data byte(s)
	long unsigned int rxId = CAN.getCanId();
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

		int frameTypeCategory = message->GetFrameTypeCategory();
		
		// messages less than 0x100 are for every node
		if (frameTypeCategory < Hapcan::Message::SystemMessage0x10Flag)
		{
			// TODO: programming messages
			OA_LOG("wiadomosc programuj¹ca < 0x100");
			return true;
		}

		//if (message->m_data[2] == 0x0 && message->m_data[3] == m_group)
		//	return false;

		//TODO: przeniesc sprawdzanie wiadomosci do receiva?

		//TODO: czy to wiaomoœæ do wszystkich w tej grupie

		// TODO czy to jest do tego noda w takim razie?


		switch (frameTypeCategory)
		{
		case Hapcan::Message::SystemMessage0x10Flag:
			ProcessMessage0x10(message);
			break;
		case Hapcan::Message::SystemMessage0x11Flag:
			//Serial.println("wiadomosc systemowa 11");
		case Hapcan::Message::NormalMessage0x30Flag:
			//Serial.println("wiadomosc normalna od urz¹dzenia");
			break;
		}
		return true;
	}
	return false;
}

// Process massage type 10
bool HapcanDevice::ProcessMessage0x10(HapcanMessage* message)
{
	
	switch(message->GetFrameType())
	{
	case Hapcan::Message::System0x10::HardwareTypeRequestToGroup: 
		if( message->m_data[2] == 0 && (message->m_data[3] == m_group || message->m_data[3] == 0x00))
			CanNodeId();
		break;
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

// Send Can Node ID
void HapcanDevice::CanNodeId()
{
	HapcanMessage message;
	message.BuildIdPart(Hapcan::Message::SystemMessage0x10Flag, Hapcan::Message::System0x10::HardwareTypeRequestToGroup, true, m_node, m_group);
	for (byte i = 0; i < 8; i++)
		message.m_data[i] = pgm_read_byte( (HARDID + i) );
	
	OA_LOG("> CanNodeId");
	message.PrintToSerial();

	CAN.sendMsgBuf(message.m_id, 1, 8, message.m_data);
}
