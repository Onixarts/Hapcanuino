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
					}

					void BuildIdPart(unsigned int frameType, bool isAnswer,  byte node, byte group ) 
					{ 
						m_id = 0;
						//m_id |= (unsigned long)frameTypeCategory << 21;
						m_id |= (unsigned long)frameType << 17;//13;
						if (isAnswer)
							m_id |= (unsigned long)1 << 16;
						m_id |= (unsigned long)node << 8;
						m_id |= group;
					}
						
					byte GetFrameTypeCategory() { return (byte) (m_id >> 21) ;}
					//byte GetFrameType() { return (byte) (((m_id >> 17) << 4 ) | ((m_id >> 16) & 0x1)); }
					unsigned int GetFrameType() { return (unsigned int)(((m_id >> 17) )); }
					bool IsAnswer() { return (bool) ((m_id >> 16) & 0x1); }
					void SetAnswer() { m_id |= (unsigned long)1 << 16; }
					byte GetNode() { return (byte) (m_id >> 8); }
					byte GetGroup() { return (byte) m_id; }
					void PrintToSerial();
			};

			typedef void(*MessageAcceptedEventDelegate)(HapcanMessage* message, byte instruction, byte param1, byte param2, byte param3);

			// this structure has to have 19 Bytes. Don't change it. It is stored in EEPROM.
			struct BoxConfigStruct
			{
				byte data[19];
				//unsigned int frameType;
				//byte senderNode;
				//byte senderGroup;
				//byte data[8];
				//byte operator_1_4;
				//byte operator_5_8;
				//byte operator_9_12;
				//byte instruction;
				//byte param1;
				//byte param2;
				//byte param3;

				bool Accept(HapcanMessage* message)
				{
					if (!Compare(0, highByte(message->GetFrameType())))
						return false;
					if (!Compare(1, lowByte(message->GetFrameType())))
						return false;
					if (!Compare(2, message->GetNode() ))
						return false;
					if (!Compare(3, message->GetGroup()))
						return false;
					for (byte i = 0; i < 8; i++)
					{
						if (!Compare(4+i, message->m_data[i]))
							return false;
					}

					return true;
				}

				bool Compare(byte byteNumber, byte messageByte)
				{
//					Serial.print(data[byteNumber], HEX);
					
					byte compareOperator = data[12 + (byteNumber / 4)];
					compareOperator = compareOperator >> ((byteNumber % 4) * 2);
					switch (compareOperator & 0x03)
					{
					case BoxOperator::Ignore:
					{
						//Serial.print(" xx ");
						//Serial.println(messageByte, HEX);
						return true;
					}
					case BoxOperator::Equal:
					{
						//Serial.print(" == ");
						//Serial.println(messageByte, HEX);

						if (data[byteNumber] == messageByte)
							return true;
					}
						break;
					case BoxOperator::Different:
					{
						//Serial.print(" != ");
						//Serial.println(messageByte, HEX);

						if (data[byteNumber] != messageByte)
							return true;
					}
						break;
//					default:
						//Serial.print("error: ");
						//Serial.println(compareOperator & 0x03);
					}
//					Serial.println(" FALSE ");
					return false;
				}
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
				bool m_isInProgrammingMode;
				bool m_isInitialized;
				unsigned int m_memoryAddress;
				byte m_memoryCommand;
				byte m_description[16];
				MessageAcceptedEventDelegate	m_messageAcceptedDelegate;
			protected:
				void AddMessageToRxBuffer(HapcanMessage& message);
				bool ProcessRxBuffer();
				bool ProcessSystemMessage(HapcanMessage * message);
				bool ProcessNormalMessage(HapcanMessage * message);
				bool ReadRxBuffer(HapcanMessage** message);
				bool MatchGroup(HapcanMessage* message);
				bool MatchNode(HapcanMessage* message);
				void ProcessProgrammingMessage(HapcanMessage* message);
				void ReadEEPROMConfig();
				void Send(HapcanMessage & message);

				void AddressFrameAction(HapcanMessage* inputMessage);
				void DataFrameAction(HapcanMessage* inputMessage);
				void ErrorFrameAction(HapcanMessage* inputMessage);
				void EnterProgrammingModeAction(unsigned int frameType);
				void ProgrammingModeAction(unsigned int frameType);
				void RebootAction();
				void CanNodeIdAction(unsigned int frameType);
				void CanFirmwareIdAction(unsigned int frameType);
				void SupplyVoltageAction(unsigned int frameType);
				void NodeDescriptionAction(unsigned int frameType);
				void SetDefaultNodeAndGroupAction(unsigned int frameType);
				void DeviceIDAction(unsigned int frameType);
				
				//void UptimeAction(unsigned int frameType);

				virtual void StatusRequestAction(HapcanMessage* message);
				virtual void ControlAction(HapcanMessage* message);
			public:
				HapcanDevice();

				void Begin(); 
				void Update();

				void ReceiveAnswerMessages(bool value) { m_receiveAnswerMessages = value; }
				
				void OnMessageAcceptedEvent(MessageAcceptedEventDelegate eventDelegate) { m_messageAcceptedDelegate = eventDelegate; }
				
				unsigned long GetRxBufferOverflowCount();

				// CAN module Interrupt handling methods. Don't use them directly.
				void OnCanReceived();
				static void OnCanReceivedDispatcher();
			};
		}
	}
}

#endif
