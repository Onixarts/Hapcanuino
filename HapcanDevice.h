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
				protected:
				boolean _isAnswer;
				int _frameType;
				byte _node;
				byte _group;
					
				public:
					byte m_frame[8];

					HapcanMessage();// : _isAnswer(false), _frameType(0), _node(0), _group(0)
					//{} 
					void Parse(unsigned long id, byte* buffer)
					{
						memcpy(m_frame, buffer, 8 );
						_node = (id & 0x0000FF00)>>8;
						_group = id & 0x000000FF;
						_frameType = (id & 0xFFFF0000) >> 17;
						//_isAnswer = !(_id & 0x400);
					}
					void Set( boolean isAnswer, uint8_t messageType, uint8_t moduleId ) 
					{ 
						//_id = 0;
						//if( isAnswer )
						//	_id &= 0 << 10;
						//else
						//	_id |= 1 << 10;
						//_id |= messageType << 5;
						//_id |= moduleId;
					}
						
					boolean IsAnswer() {return _isAnswer;}
					int GetFrameType() { return _frameType;}
					byte GetNode() {return _node;}
					byte GetGroup() { return _group; }
					void PrintToSerial();
			};

			class HapcanDevice
			{
				MCP_CAN CAN;
				volatile byte m_RxBufferIndex;
				byte m_RxBufferReadIndex;
				HapcanMessage m_RxBuffer[Config::RxFifoQueueSize];
				unsigned int m_rxBufferOverflowCount;
			protected:
				void AddMessageToRxBuffer(HapcanMessage& message);
				
			public:
				HapcanDevice();

				void Begin(); 
				void OnCanReceived();
				
				bool ReadRxBuffer(HapcanMessage** message);
				unsigned long GetRxBufferOverflowCount();

				static void OnCanReceivedDispatcher();
			};
		}
	}
}

#endif
