/*!
 * @file ShieldCommunication.h
 * @brief Software for IO-Link shield usage, main function
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

//!**** Header-Files ************************************************************
// ToDo: Check if all includes are necessary
#include <cstdio>
#include <csignal>
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <any>
#include "HardwareRaspberry.h"
#include "crow_all.h"
#include "Max14819.h"
#include "IOLMasterPort.h"
#include "IOLMasterPortMax14819.h"
#include "IOLGenericDevice.h"
#include "IOLink.h"
#include <string>
//!**** Functions **********************************************************
using namespace std; // toDo: replace
class ShieldCommunication {
private:
    HardwareRaspberry hardware;
    IoddManager instance;
    vector<IOLMasterPortMax14819> ports;
    vector<int> port_nr;
    vector<uint8_t> pData;
    map<string, uint8_t> pData_ports;
    bool extended_board = false;
    int cycleTime=100;
    int timeSinceEpochMillisec();
    void Communication_startup(bool extended_board);
    void Communication_shutdown();
    string mqtt_IP_string;
    const char* mqtt_IP;
public:
    mutex max1Mutex;
    mutex max2Mutex;
    void signalHandler(int signum);
    ShieldCommunication(bool extended_board);
    ~ShieldCommunication();
    void Read_port(uint8_t port_nr);
    //void Read_all_ports();
    //void Write_all_ports();
    void PD_all_ports();
    void send_all_PD();
    vector<uint8_t> get_PD_portx(string port);
    void ISDU_Write(uint8_t port_nr, uint16_t index, uint8_t subIndex, vector<uint8_t> pData);
    void ISDU_Read(uint8_t port_nr, uint16_t index, uint8_t subIndex, vector<uint8_t> &Data);
    void Write_Port(uint8_t port_nr);
    void Write_procDataOut(uint8_t port_nr, vector<uint8_t> pData);
    void writeCycleTime(int time_in_ms);
    void isDeviceConnected(vector<uint8_t>& portConnection);
    int getCycleTime();
    void writeIP(string newIP);
};
int timeSinceEpochMillisec();
