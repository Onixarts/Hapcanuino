#pragma once
// ==============================================================================================================
// === LOCAL DEFINITION =========================================================================================
// ==============================================================================================================
#define     HARD1   0x4F       //hardware type (0x4F41 - OA)
#define     HARD2   0x41       //hardware type
#define     HVERS   .01        //hardware version (OA 1)

#define     BVER    .01        //bootloader software major version
#define     BREV    .00        //bootloader software minor version
// ==============================================================================================================
// === NODE SERIAL NUMBER =======================================================================================
// ==============================================================================================================
#define     ID0     0x00            //node serial number MSB
#define     ID1     0x00            //node serial number
#define     ID2     0x00            //node serial number
#define     ID3     0x01            //node serial number LSB
// ==============================================================================================================
// === HARDWARE ID ==============================================================================================
// ==============================================================================================================
const byte HARDID[] PROGMEM = { HARD1, HARD2, HVERS, 0xFF, ID0, ID1, ID2, ID3 };

namespace Onixarts
{
	namespace HomeAutomationCore
	{
		namespace Hapcan
		{
			namespace Config
			{
				const byte RxFifoQueueSize = 10;				// max 255

				namespace MCP
				{
					const byte InterruptPin = 2;				// CAN module interrupt is connected to this pin (see https://www.arduino.cc/en/Reference/AttachInterrupt)
					const byte CSPin = 10;						// SPI CS pin
					const byte OscillatorFrequency = MCP_8MHz;	// MCP oscillator frequency on MCP CAN module (or MCP_16MHz)
				}
			}
		}
	}
}
