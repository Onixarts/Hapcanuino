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
#include "Consts.h"

namespace Onixarts 
{ 
	namespace HomeAutomationCore 
	{
		namespace Hapcan 
		{
			class Tools
			{
			public:
				static unsigned long Byte2Time(byte time)
				{
					if (time < 60)
						return 1000 * (unsigned long)time;
					else if (time < 108)
						return 1000 * (60 + ((unsigned long)time - 60) * 5);
					else if (time < 163)
						return 1000 * (300 + ((unsigned long)time - 108) * 60);
					else
						return 1000 * (3600 + ((unsigned long)time - 163) * 900);
				}
			};

			class HapcanMessage
			{
					void InitMessageId(unsigned int frameType, bool isAnswer, byte node, byte group)
					{
						m_id = 0;
						m_id |= (unsigned long)frameType << 17;
						if (isAnswer)
							m_id |= (unsigned long)1 << 16;
						m_id |= (unsigned long)node << 8;
						m_id |= group;
					}
				public:
					unsigned long m_id;
					byte m_data[8];

					HapcanMessage();
					HapcanMessage(unsigned long id, byte* buffer);
					HapcanMessage(unsigned int frameType, bool isAnswer, byte node, byte group);
					HapcanMessage(unsigned int frameType, bool isAnswer);
					

					// Fills node and group number if needed
					void Prepare(byte node, byte group)
					{
						if( GetNode() == 0 )
							m_id |= (unsigned long)node << 8;
						if (GetGroup() == 0)
							m_id |= group;
					}
						
					byte GetFrameTypeCategory() { return (byte) (m_id >> 21) ;}
					unsigned int GetFrameType() { return (unsigned int)(((m_id >> 17) )); }
					bool IsAnswer() { return (bool) ((m_id >> 16) & 0x1); }
					void SetAnswer() { m_id |= (unsigned long)1 << 16; }
					byte GetNode() { return (byte) (m_id >> 8); }
					byte GetGroup() { return (byte) m_id; }
					void PrintToSerial();
			};

			typedef void(*ExecuteInstructionDelegate)(byte instruction, byte param1, byte param2, byte param3, HapcanMessage& message);
			typedef void(*StatusRequestDelegate)(byte requestType, bool isAnswer);

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
					if (!Compare(0, (highByte(message->GetFrameType())) << 4))
						return false;
					if (!Compare(1, (lowByte(message->GetFrameType())) << 4))
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
					byte compareOperator = data[12 + (byteNumber / 4)];
					compareOperator = compareOperator >> ((byteNumber % 4) * 2);
					switch (compareOperator & 0x03)
					{
					case BoxOperator::Ignore:
					{
						return true;
					}
					case BoxOperator::Equal:
					{
						if (data[byteNumber] == messageByte)
							return true;
					}
						break;
					case BoxOperator::Different:
					{
						if (data[byteNumber] != messageByte)
							return true;
					}
						break;
					}
					return false;
				}
			};

			class HapcanDevice
			{
				MCP_CAN CAN;
				volatile byte m_RxBufferIndex;
				byte m_TxBufferIndex;
				byte m_RxBufferReadIndex;
				byte m_TxBufferReadIndex;
				HapcanMessage m_RxBuffer[Config::RxFifoQueueSize];
				HapcanMessage m_TxBuffer[Config::TxFifoQueueSize];
				unsigned int m_rxBufferOverflowCount;
				unsigned int m_txBufferOverflowCount;
				byte m_group;
				byte m_node;
				bool m_processOwnMessages;
				bool m_receiveAnswerMessages;
				bool m_isInProgrammingMode;
				bool m_isInitialized;
				unsigned int m_memoryAddress;
				byte m_memoryCommand;
				unsigned long m_uptime;
				unsigned long m_lastMillis;
				ExecuteInstructionDelegate m_executeInstructionDelegate;
				StatusRequestDelegate m_statusRequestDelegate;
			protected:
				void AddMessageToRxBuffer(HapcanMessage& message);
				void AddMessageToTxBuffer(HapcanMessage& message);
				bool ProcessRxBuffer();
				bool ProcessTxBuffer();
				void UpdateUptime();
				bool ProcessSystemMessage(HapcanMessage * message);
				bool ProcessNormalMessage(HapcanMessage * message);
				bool ReadRxBuffer(HapcanMessage** message);
				bool ReadTxBuffer(HapcanMessage ** message);
				bool MatchGroup(HapcanMessage* message);
				bool MatchNode(HapcanMessage* message);
				void ProcessProgrammingMessage(HapcanMessage* message);
				void ReadEEPROMConfig();

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
				void UptimeAction(unsigned int frameType);
				void StatusRequestAction(HapcanMessage* message);
				void ControlAction(HapcanMessage* message);

				virtual void OnInit() {};
				virtual void OnReadEEPROMConfig() {};
				virtual void OnUpdate() {};
				virtual void OnExecuteInstruction(byte instruction, byte param1, byte param2, byte param3, Hapcan::HapcanMessage& message) {};
				virtual void OnStatusRequest(byte requestType, bool isAnswer) {};

			public:
				HapcanDevice();


				void Begin(); 
				void Update();
				void Send(HapcanMessage & message, bool sendImmediately = false);

				void ReceiveAnswerMessages(bool value = true) { m_receiveAnswerMessages = value; }
				void ProcessOwnMessages(bool value = true) { m_processOwnMessages = value; }
				bool GetConfigByte(byte configBank, byte byteNumber, byte& value);
				bool SetConfigByte(byte configBank, byte byteNumber, byte value);
				
				void SetExecuteInstructionDelegate(ExecuteInstructionDelegate eventDelegate) { m_executeInstructionDelegate = eventDelegate; }
				void SetStatusRequestDelegate(StatusRequestDelegate eventDelegate) { m_statusRequestDelegate = eventDelegate; }
				
				unsigned long GetRxBufferOverflowCount();

				// CAN module Interrupt handling methods. Don't use them directly.
				void OnCanReceived();
				static void OnCanReceivedDispatcher();
			};

			class SubModuleBase
			{
			protected:
				byte m_instructionShift;
				virtual bool Execute(byte instruction, byte param1, byte param2, byte param3, Hapcan::HapcanMessage& message) = 0;
			public:
				SubModuleBase(byte instructionShift) : m_instructionShift(instructionShift) {};
				virtual void Init() = 0;
				virtual void Update() = 0;
				virtual void SendStatus(bool isAnswer) = 0;
				bool ExecuteInstruction(byte instruction, byte param1, byte param2, byte param3, Hapcan::HapcanMessage& message)
				{
					return Execute(instruction - m_instructionShift, param1, param2, param3, message);
				}
			};

			template <byte SubmodulesCount = 0>
			class HapcanDeviceSubModuleHost : public HapcanDevice
			{
			protected:
				SubModuleBase* m_subModules[SubmodulesCount];

				virtual void OnInit()
				{
					for (byte i = 0; i < SubmodulesCount; i++)
					{
						m_subModules[i]->Init();
					}
				}

				virtual void OnUpdate()
				{
					for (byte i = 0; i < SubmodulesCount; i++)
					{
						m_subModules[i]->Update();
					}
				}

				virtual void OnStatusRequest(byte requestType, bool isAnswer)
				{
					for (byte i = 0; i < SubmodulesCount; i++)
					{
						m_subModules[i]->SendStatus(isAnswer);
					}
				}
				
				virtual void OnExecuteInstruction(byte instruction, byte param1, byte param2, byte param3, Hapcan::HapcanMessage& message)
				{
					for (byte i = 0; i < SubmodulesCount; i++)
					{
						m_subModules[i]->ExecuteInstruction(instruction, param1, param2, param3, message);
					}
				}

			public:
				HapcanDeviceSubModuleHost()
				{
					if( SubmodulesCount > 0 )
						memset(m_subModules, 0, sizeof(m_subModules));
				};
			};
			
		}
	}
}

#endif
