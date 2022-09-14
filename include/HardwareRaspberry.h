/*!
 * @file HardwareRaspberry.h
 * @brief Generic Hardware Layer abstraction of a physical layer
 * @copyright 2022 Balluff GmbH
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *	    http://www.apache.org/licenses/LICENSE-2.0
 *
 *	 Unless required by applicable law or agreed to in writing, software
 *	 distributed under the License is distributed on an "AS IS" BASIS,
 *	 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	 See the License for the specific language governing permissions and
 *	 limitations under the License.
 * @author See AUTHORS file
 * @since 11.08.2022
 */

#ifndef _HARDWARRASPBERRY_H
#define _HARDWARRASPBERRY_H

//!**** Header-Files ************************************************************
#include <cstdint>

//!**** Implementation **********************************************************

class HardwareRaspberry {
public:
	HardwareRaspberry();
	~HardwareRaspberry();

    enum PinMode { out, in_pullup, in };
	enum PinNames {port01CS, port23CS, port01IRQ, port23IRQ, port0DI, port1DI, port2DI, port3DI,
	port0LedGreen, port0LedRed, port0LedRxErr, port0LedRxRdy,
	port1LedGreen, port1LedRed, port1LedRxErr, port1LedRxRdy,
	port2LedGreen, port2LedRed, port2LedRxErr, port2LedRxRdy,
	port3LedGreen, port3LedRed, port3LedRxErr, port3LedRxRdy
	};

	virtual void begin();	
	virtual void IO_Write(PinNames pinnumber, uint8_t state);
	virtual void IO_PinMode(PinNames pinnumber, PinMode mode); //pinMode
	virtual void Serial_Write(char const * buf);
	virtual void Serial_Write(int number);
	virtual void SPI_Write(uint8_t channel, uint8_t * data, uint8_t length);
	virtual void wait_for(uint32_t delay_ms);

private:
	uint8_t get_pinnumber(PinNames pinname);
};

#endif //_HARDWARRASPBERRY_H
