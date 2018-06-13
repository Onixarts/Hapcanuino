#ifndef _DIGITALINPUT_h
#define _DIGITALINPUT_h

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
				namespace DigitalInput
				{
					namespace Status
					{
						const byte Released = 0x00;
						const byte Pressed = 0xFF;
					}

					namespace EventNotification
					{
						const byte Pressed = 0x01;
						const byte Released = 0x40;
					}

					class Module : public IO::SimpleDigitalInput, public Hapcan::SubModuleBase
					{
						byte m_standardEventNotification;
						byte m_extendedEventNotification;
						byte m_channel;
						Hapcan::HapcanDevice& m_device;
					public:
						Module(Hapcan::HapcanDevice& device, byte channel, byte inputPin, bool enablePullUpResistor = false, byte inputActiveLevel = HIGH/*, byte instructionShift*/)
							: Hapcan::SubModuleBase(0)
							, IO::SimpleDigitalInput(inputPin, enablePullUpResistor, inputActiveLevel)
							, m_channel(channel)
							, m_device(device) 
							, m_standardEventNotification(0xFF)
						{}

						void Init()
						{
							IO::SimpleDigitalInput::Init();
						}

						void Update()
						{
							IO::SimpleDigitalInput::Update();
						}

						void SendStatus(bool isAnswer)
						{
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, isAnswer);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] =  IsPressed() ? 0xFF : 0x00;
							m_device.Send(statusMessage);
						}

						bool Execute(InstructionStruct& exec, Hapcan::HapcanMessage& message)
						{
							return false;
						}

						void SetStandardEventNotifications(byte eventNotificationConfig)
						{
							m_standardEventNotification = eventNotificationConfig;
						}

						void OnPressed() 
						{
							if (!(m_standardEventNotification & EventNotification::Pressed))
								return;

							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::Pressed;
							m_device.Send(statusMessage);
						}
						void OnReleased()
						{
							if (!(m_standardEventNotification & EventNotification::Released))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::Released;
							m_device.Send(statusMessage);
						}
					};
				}
			}
		}
	}
}
#endif

