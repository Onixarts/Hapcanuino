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


			struct InstructionStruct
			{
				// this structure has to have 32 Bytes. Don't change it. It is stored in EEPROM. See /docs/Hapcanuino_1-50-0-0-memory.xlsx for details
				byte data[32];

				byte Instruction() { return data[24]; }
				byte Parameter1() { return data[25]; }
				byte Parameter2() { return data[26]; }
				byte Parameter3() { return data[27]; }
				byte Parameter4() { return data[28]; }
				byte Parameter5() { return data[29]; }
				byte Parameter6() { return data[30]; }
				byte Parameter7() { return data[31]; }

				void TranslateInstruction(byte shift) { data[24] -= shift; }
				void InitFromBytes(byte instruction, byte param1, byte param2, byte param3, byte param4, byte param5)
				{
					data[24] = instruction;
					data[25] = param1;
					data[26] = param2;
					data[27] = param3;
					data[28] = param4;
					data[29] = param5;
					data[30] = 0xFF;
					data[31] = 0xFF;
				}
			};
			struct BoxConfigStruct : InstructionStruct
			{
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
					byte compareOperator = data[12 + byteNumber];
					switch (compareOperator)
					{
					case BoxOperator::Ignore:
						return true;
					case BoxOperator::Equal:
						return (data[byteNumber] == messageByte);
					case BoxOperator::Different:
						return (data[byteNumber] != messageByte);
					case BoxOperator::LessOrEqual:
						return (data[byteNumber] >= messageByte);
					case BoxOperator::GraterOrEqual:
						return (data[byteNumber] <= messageByte);
					}
					return false;
				}

			};

			typedef void(*ExecuteInstructionDelegate)(InstructionStruct& exec, HapcanMessage& message);
			typedef void(*StatusRequestDelegate)(byte requestType, bool isAnswer);

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
				virtual void OnExecuteInstruction(InstructionStruct& exec, Hapcan::HapcanMessage& message) {};
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
				virtual bool Execute(InstructionStruct& exec, Hapcan::HapcanMessage& message) = 0;
			public:
				SubModuleBase(byte instructionShift) : m_instructionShift(instructionShift) {};
				virtual void Init() = 0;
				virtual void Update() = 0;
				virtual void SendStatus(bool isAnswer) = 0;
				bool ExecuteInstruction(InstructionStruct& exec, Hapcan::HapcanMessage& message)
				{
					exec.TranslateInstruction(m_instructionShift);
					return Execute(exec, message);
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
				
				virtual void OnExecuteInstruction(InstructionStruct& exec, Hapcan::HapcanMessage& message)
				{
					for (byte i = 0; i < SubmodulesCount; i++)
					{
						m_subModules[i]->ExecuteInstruction(exec, message);
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
