#ifndef _HAPCANBUTTON_h
#define _HAPCANBUTTON_h

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
				namespace HapcanButton
				{
					namespace Status
					{
						const byte Released = 0x00;
						const byte Pressed = 0xFF;
						const byte Held400ms = 0xFE;
						const byte Held4s = 0xFD;
						const byte ReleasedBefore400ms = 0xFC;
						const byte ReleasedAfter400ms = 0xFB;
						const byte ReleasedAfter4s = 0xFA;

						// Hapcanuino extensions
						const byte Held1s = 0xF1;
						const byte ReleasedAfter1s = 0xF0;
					}

					namespace EventNotification
					{
						const byte Pressed = 0x01;
						const byte ReleasedBefore400ms = 0x02;
						const byte Held400ms = 0x04;
						const byte ReleasedAfter400ms = 0x08;
						const byte Held4s = 0x10;
						const byte ReleasedAfter4s = 0x20;
						const byte Released = 0x40;

						const byte Held1s = 0x01;
						const byte ReleasedAfter1s = 0x02;
					}

					class Module : public IO::DigitalInput, public Hapcan::SubModuleBase
					{
						byte m_standardEventNotification;
						byte m_extendedEventNotification;
						byte m_channel;
						Hapcan::HapcanDevice& m_device;
					public:
						Module(Hapcan::HapcanDevice& device, byte channel, byte inputPin, bool enablePullUpResistor = false, byte inputActiveLevel = HIGH/*, byte instructionShift*/)
							: Hapcan::SubModuleBase(0)
							, IO::DigitalInput(inputPin, enablePullUpResistor, inputActiveLevel)
							, m_channel(channel)
							, m_device(device) 
							, m_standardEventNotification(0xFF)
							, m_extendedEventNotification(0xFF)
						{}

						void Init()
						{
							IO::DigitalInput::Init();
						}

						void Update()
						{
							IO::DigitalInput::Update();
						}

						void SendStatus(bool isAnswer)
						{
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, isAnswer);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] =  IsPressed() ? 0xFF : 0x00;
							m_device.Send(statusMessage);
						}

						bool Execute(byte instruction, byte param1, byte param2, byte param3, Hapcan::HapcanMessage& message)
						{
							return false;
						}

						void SetStandardEventNotifications(byte eventNotificationConfig)
						{
							m_standardEventNotification = eventNotificationConfig;
						}

						void SetExtendedEventNotifications(byte eventNotificationConfig)
						{
							m_extendedEventNotification = eventNotificationConfig;
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
						
						void OnReleasedBefore400ms()
						{
							if (!(m_standardEventNotification & EventNotification::ReleasedBefore400ms))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::ReleasedBefore400ms;
							m_device.Send(statusMessage);
						}
						
						void OnHeld400ms()
						{
							if (!(m_standardEventNotification & EventNotification::Held400ms))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::Held400ms;
							m_device.Send(statusMessage);
						}
						
						void OnReleasedAfter400ms()
						{
							if (!(m_standardEventNotification & EventNotification::ReleasedAfter400ms))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::ReleasedAfter400ms;
							m_device.Send(statusMessage);
						}
						
						void OnHeld1s()
						{
							if (!(m_extendedEventNotification & EventNotification::Held1s))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::Held1s;
							m_device.Send(statusMessage);
						}

						void OnReleasedAfter1s()
						{
							OnReleasedAfter400ms();

							if (!(m_extendedEventNotification & EventNotification::ReleasedAfter1s))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::ReleasedAfter1s;
							m_device.Send(statusMessage);
						}
						
						void OnHeld4s()
						{
							if (!(m_standardEventNotification & EventNotification::Held4s))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::Held4s;
							m_device.Send(statusMessage);
						}
						
						void OnReleasedAfter4s()
						{
							if (!(m_standardEventNotification & EventNotification::ReleasedAfter4s))
								return;
							Hapcan::HapcanMessage statusMessage(Hapcan::Message::Normal::ButtonNodeMessage, false);
							statusMessage.m_data[2] = m_channel;
							statusMessage.m_data[3] = Status::ReleasedAfter4s;
							m_device.Send(statusMessage);
						}
					};
				}
			}
		}
	}
}
#endif

