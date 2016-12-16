#pragma once

// Send debug info to Serial port. Comment line below to disable Serial notifications and reduce FLASH and SRAM usage
#define OA_DEBUG 1	

#ifdef OA_DEBUG
#define OA_LOG(text) Serial.println(text)
#define OA_LOG_LINE(text) Serial.print(text)
#else
#define OA_LOG(text) //empty
#define OA_LOG_LINE(text) //empty
#endif	

namespace Onixarts
{
	namespace HomeAutomationCore
	{
		namespace Hapcan
		{
			// Do not modify config here. You should modify extern const in ino file
			namespace Config
			{
				const byte RxFifoQueueSize = 20;				// max 255
				
				namespace Hardware
				{
					const byte HardwareType1 = 0x4f;			//hardware type (0x3000 = UNIV 3, 0x4F41 = OA Hapcanuino)
					const byte HardwareType2 = 0x41;
					const byte HardwareVersion = 1;				// 3 for Hapcan's UNIV 3, 1 = Arduino Uno 

					extern const byte DeviceId1;// = 0x12;				//* unique device identifier 1, change it
					extern const byte DeviceId2;// = 0x34;				//* unique device identifier 2, change it
				}

				namespace Node
				{
					extern const byte SerialNumber0;// = 0x01;			// ID0 serial number MSB
					extern const byte SerialNumber1;// = 0x02;
					extern const byte SerialNumber2;// = 0x20;			// this is also a default node
					extern const byte SerialNumber3;// = 0x09;			// this is also a default group
				}

				namespace Firmware
				{
					extern const byte ApplicationType;// = 50;			//* application (hardware) type (such as button, relay, dimmer) 1-10 Hapcan modules, 102 - ethernet, 50 - default Hapcanuino empty device
					extern const byte ApplicationVersion;// = 0;			//* application (hardware) version, 0 - default Hapcanuino empty device
					extern const byte FirmwareVersion;// = 0;				//* firmware version
					extern const int FirmwareRevision;// = 0;				//* firmware revision
				}

				namespace MCP
				{
					extern const byte InterruptPin;// = 2;				//* CAN module interrupt is connected to this pin (see https://www.arduino.cc/en/Reference/AttachInterrupt)
					extern const byte CSPin;// = 10;						//* SPI CS pin
					extern const byte OscillatorFrequency;// = MCP_8MHZ;	//* MCP oscillator frequency on MCP CAN module (or MCP_16MHz)
				}

				namespace BootLoader
				{
					const byte BootloaderVersion = 1;			// BVER1
					const byte BootloaderRevision = 0;			// BREV2
				}
			}

			namespace Programming
			{
				namespace Command
				{
					const byte Undefined = 0x00;
					const byte Read = 0x01;
					const byte Write = 0x02;
					const byte Erase = 0x03;
				}
				
			}

			// Do not modiffy CoreConfig, unless You know what are You doing ;). Go find another config.
			namespace CoreConfig
			{
				const byte BoxCount = 32; // stick to 2^n

				namespace EEPROM
				{
					const byte DescriptionAddress = 0x30;
					const byte BoxEnableAddress = 0x40;
					const byte BoxConfigAddress = 0x80;
				}
			}
			
			namespace BoxOperator 
			{
				const byte Ignore = 0x00;
				const byte Equal = 0x01;
				const byte Different = 0x02;
			}
			
			namespace Message
			{
				const byte NormalMessageCategory = 0x30;

				namespace System
				{
					// Handled by bootloader in programming mode
					const unsigned int ExitAllFromBootloaderProgrammingMode = 0x010;
					const unsigned int ExitOneNodeFromBootloaderProgrammingMode = 0x020;
					const unsigned int AddressFrame = 0x030;
					const unsigned int DataFrame = 0x040;

					const unsigned int ErrorFrame = 0x0F0;

					// Handled by bootloader
					const unsigned int EnterProgrammingMode = 0x100;
					const unsigned int RebootRequestToGroup = 0x101;
					const unsigned int RebootRequestToNode = 0x102;
					const unsigned int HardwareTypeRequestToGroup = 0x103;
					const unsigned int HardwareTypeRequestToNode = 0x104;
					const unsigned int FirmwareTypeRequestToGroup = 0x105;
					const unsigned int FirmwareTypeRequestToNode = 0x106;
					const unsigned int SetDefaultNodeAndGroupRequestToNode = 0x107;

					// Handled by functional firmware
					const unsigned int StatusRequestToGroup = 0x108;
					const unsigned int StatusRequestToNode = 0x109;
					namespace StatusRequestType
					{
						const byte SendAll = 0x0;
					}
					const unsigned int ControlMessage = 0x10A;

					//Handled by bootloader
					const unsigned int SupplyVoltageRequestToGroup = 0x10B;
					const unsigned int SupplyVoltageRequestToNode = 0x10C;
					const unsigned int DescriptionRequestToGroup = 0x10D;
					const unsigned int DescriptionRequestToNode = 0x10E;
					const unsigned int DeviceIDRequestToGroup = 0x10F;

					const unsigned int DeviceIDRequestToNode = 0x111;
					const unsigned int UptimeRequestToGroup = 0x112;
					const unsigned int UptimeRequestToNode = 0x113;
					const unsigned int HealthCheckRequestToGroup = 0x114;
					const unsigned int HealthCheckRequestToNode = 0x115;
				}

				namespace Normal
				{
					// Handled by functional firmware
					const unsigned int ButtonNodeMessage = 0x301;
					const unsigned int RelayMessage = 0x302;
					const unsigned int InfraredReceiverMessage = 0x303;
					const unsigned int TemperatureSensorMessage = 0x304;
					const unsigned int InfraredTransmitterMessage = 0x305;
					const unsigned int DimmerMessage = 0x306;
					const unsigned int BlindControllerMessage = 0x307;
					const unsigned int LedControllerMessage = 0x308;
				}
			}

			//namespace Control
			//{
			//	namespace Box
			//	{
			//		namespace Value 
			//		{
			//			const byte Enable = 0xDD;
			//			const byte Disable = 0xDE;
			//			const byte Toggle = 0xDF;
			//		}
			//		const byte Instruction = 0;
			//		const byte BoxX = 1;
			//		const byte BoxY = 2;
			//	}
			//}
		}
	}
}
