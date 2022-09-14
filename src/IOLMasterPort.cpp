/*!
 * @file IOLMasterPort.cpp
 * @brief Abstract class layer for driver ic independent port implementation. Defines methods 
 *        and variables for the IOLMasterPort<driver_ic> implementation.
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

//!***** Header-Files//!**********************************************************
#include "IOLMasterPort.h"

//!*******************************************************************************
//!  function :    IOLMasterPort
//!*******************************************************************************
//!  \brief        Constructor for IOLMasterPort
//!
//!  \type         local
//!
//!  \param[in]    void
//!
//!  \return       void
//!
//!*******************************************************************************
IOLMasterPort::IOLMasterPort()
:portType_(0),
diModeSupport_(0),
portMode_(0),
portStatus_(0),
actualCycleTime_(0),
comSpeed_(0)
{

}

IOLMasterPort::~IOLMasterPort()
{
}
