#include "Arduino.h"
#include "HapcanDevice.h"

using namespace Onixarts::HomeAutomationCore::Hapcan;

HapcanMessage::HapcanMessage()
: _isAnswer(false), _frameType(0), _node(0), _group(0)
{}

void HapcanMessage::PrintToSerial()
{
	Serial.print("Frame: ");
	Serial.print(GetFrameType(), HEX);

	Serial.print(" Node: ");
	Serial.print(GetNode());

	Serial.print(" Gr: ");
	Serial.print(GetGroup());

	Serial.print(" msg: ");
	for (int i = 0; i<8; i++)                // Print each byte of the data
	{
		if (m_frame[i] < 0x10)                     // If data byte is less than 0x10, add a leading zero
		{
			Serial.print("0");
		}
		Serial.print(m_frame[i], HEX);
		Serial.print(" ");
	}
	Serial.println();
}

HapcanDevice* pHapcanDevice = NULL;

HapcanDevice::HapcanDevice()
	:CAN(Config::MCP::CSPin)
	, m_RxBufferIndex(0)
	, m_RxBufferReadIndex(0)
	, m_rxBufferOverflowCount(0)
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
	if (m_RxBufferIndex == Config::RxFifoQueueSize)
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
	if (m_RxBufferReadIndex == Config::RxFifoQueueSize)
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

	if (hapcanMessage.IsAnswer())				// ignore answer messages
		return;
	
	AddMessageToRxBuffer(hapcanMessage);
}

// Returns RX FIFO buffer overflow count
unsigned long HapcanDevice::GetRxBufferOverflowCount()
{
	return m_rxBufferOverflowCount;
}