/*
  Onixarts Hapcan library for Arduino
  Author:  Bartosz Rosa
  Version: 1.0
  License: Free to use.
  mail: software@onixarts.pl
*/

#ifndef _OnixartsHapcanuino_h
#define _OnixartsHapcanuino_h

#include <mcp_can.h>
#include "Arduino.h"
#include "DeviceConfig.h"

namespace Onixarts 
{ 
	namespace HomeAutomationCore 
	{
		namespace Hapcan 
		{
			class HapcanMessage
			{
				public:
					unsigned long m_id;
					byte m_data[8];

					HapcanMessage();
					void Parse(unsigned long id, byte* buffer)
					{
						m_id = id;
						memcpy(m_data, buffer, 8 );

						//_node = (id & 0x0000FF00)>>8;
						//_group = id & 0x000000FF;
						//_frameType = (id & 0xFFFF0000) >> 17;
						//_isAnswer = !(_id & 0x400);
					}

					void BuildIdPart( byte frameTypeCategory, byte frameType, bool isAnswer,  byte node, byte group ) 
					{ 
						m_id = 0;
						m_id |= (unsigned long)frameTypeCategory << 21;
						m_id |= (unsigned long)frameType << 13;
						if (isAnswer)
							m_id |= (unsigned long)1 << 16;
						m_id |= (unsigned long)node << 8;
						m_id |= group;
					}
						
					byte GetFrameTypeCategory() { return (byte) (m_id >> 21) ;}
					byte GetFrameType() { return (byte) (((m_id >> 17) << 4 ) | ((m_id >> 16) & 0x1)); }
					bool IsAnswer() { return (bool) ((m_id >> 16) & 0x1); }
					byte GetNode() { return (byte) (m_id >> 8); }
					byte GetGroup() { return (byte) m_id; }
					void PrintToSerial();
			};

			class HapcanDevice
			{
				MCP_CAN CAN;
				volatile byte m_RxBufferIndex;
				byte m_RxBufferReadIndex;
				HapcanMessage m_RxBuffer[Config::RxFifoQueueSize];
				unsigned int m_rxBufferOverflowCount;
				byte m_group;
				byte m_node;
				bool m_receiveAnswerMessages;
			protected:
				void AddMessageToRxBuffer(HapcanMessage& message);
				bool ProcessRxBuffer();
				bool ProcessMessage0x10(HapcanMessage * message);
				bool ReadRxBuffer(HapcanMessage** message);

				void CanNodeId();
			public:
				HapcanDevice();

				void Begin(); 
				void ReceiveAnswerMessages(bool value) { m_receiveAnswerMessages = value; }
				void OnCanReceived();

				void Update();
				
				unsigned long GetRxBufferOverflowCount();

				static void OnCanReceivedDispatcher();
			};
		}
	}
}

#endif
