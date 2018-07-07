// HapcanRelay.h

#ifndef _HAPCANRELAY_h
#define _HAPCANRELAY_h

#include "arduino.h"
#include "../Onixarts_Hapcanuino/HapcanDevice.h"
#include <OnixartsIO.h>

using namespace Onixarts::HomeAutomationCore;
using namespace Onixarts::Tools;

namespace Onixarts
{
	namespace HomeAutomationCore
	{
		namespace Hapcan
		{
			namespace SubModule
			{
				namespace HapcanRelay
				{
					namespace Instruction
					{
						const byte TurnOff = 0x00;
						const byte TurnOn = 0x01;
						const byte Toggle = 0x02;

						// Hapcanuino extensions
						const byte Blink = 0x05; 
					}

					class Module : public IO::DigitalOutput, public Hapcan::SubModuleBase
					{
						byte m_channel;
						Hapcan::HapcanDevice& m_device;
					public:
						Module(Hapcan::HapcanDevice& device, byte channel, byte outputPin, byte instructionShift)
							: Hapcan::SubModuleBase(instructionShift)
							, IO::DigitalOutput(outputPin)
							, m_channel(channel)
							, m_device(device) 
						{}

						void OnStateChanged()
						{
							SendStatus(false);
						}

						void Init()
						{
							IO::DigitalOutput::Init();
						}

						void Update()
						{
							IO::DigitalOutput::Update();
						}

						void SendStatus(bool isAnswer)
						{
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::RelayMessage, isAnswer);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = IsActive() ? 0xFF : 0x00;
							m_device.Send(statusMessage);
						}

						bool Execute(InstructionStruct& exec, Hapcan::HapcanMessage& message)
						{
							if (exec.Parameter1() != 0xFF && !(exec.Parameter1() & (1 << (m_channel - 1))))
								return false;
							
							unsigned long delay = Hapcan::Tools::Byte2Time(exec.Parameter2());

							switch (exec.Instruction())
							{
							case Instruction::TurnOff: Off(delay); return true;
							case Instruction::TurnOn: On(delay); return true;
							case Instruction::Toggle: Toggle(delay); return true;
							case Instruction::Blink:
							{  
								// blink, channel, X, X, delay, repeat count, duration
								unsigned long duration = Hapcan::Tools::Byte2Time(message.m_data[6]);
								Blink(duration == 0 ? 1000 : duration, exec.Parameter3(), delay);
								return true;
							}
							}
							return false;
						}
					};
				}
			}
		}
	}
}
#endif

