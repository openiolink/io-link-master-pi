/*!
 * @file IOLMasterPortMax14819.h
 * @brief Implementation of IOMasterPort class for the driver IC Maxim MAX14819. Extends IOMasterPort class.
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

#ifndef IOLMASTERPORTMAX14819_H_INCLUDED
#define IOLMASTERPORTMAX14819_H_INCLUDED

//!***** Header-Files ***********************************************************
#include "IOLMasterPort.h"
#include "Max14819.h"
#include <stdint.h>
#include <tuple>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "IOLink.h"
#include "IoddManager.h"
using namespace std; //toDo replace
using json = nlohmann::json;

//!***** Implementation *********************************************************

class PDclass{
private:
    vector<uint8_t> procData;
    vector<uint8_t> procDataOut;
    uint16_t VendorID;
    uint32_t DeviceID;
    uint8_t iolRev;
    std::tuple<bool, uint16_t, uint16_t> condition;
public:
    PDclass();
    ~PDclass();
    void write_pd_storage(vector<uint8_t>PData);
    void write_procDataOut(vector<uint8_t>PDout);
    vector<float>get_float(uint8_t length);
    vector<uint8_t>get_uint8_t(uint8_t length);
    vector<uint8_t>get_procDataOut();
    nlohmann::json interpretProcessData(IoddManager& instance);
    void set_iodd(uint16_t VendorID_, uint32_t DeviceID_, uint8_t RevisionID_);
};

class IOLMasterPortMax14819: public IOLMasterPort{
private:
    max14819::Max14819* pDriver_;
    max14819::PortSelect port_;
    uint16_t portType_;
    uint16_t diModeSupport_;
    uint16_t portMode_;
    uint16_t portStatus_;
    uint16_t actualCycleTime_;
    uint32_t comSpeed_;
    uint16_t VendorID_;
    uint32_t DeviceID_;
    uint8_t RevisionID_;
    uint8_t mSequenceType_;
    uint8_t ProcessDataIn_ = 0;
    uint8_t ProcessDataOut_ = 0;
    uint8_t ProcessDataInByte_;
    uint8_t ProcessDataOutByte_;
    uint8_t OnRequestData_ = 0;
    PDclass pdclass;
    bool deviceConnection=0;

public:
    IOLMasterPortMax14819();
    IOLMasterPortMax14819(max14819::Max14819* pDriver, max14819::PortSelect port);
    ~IOLMasterPortMax14819();
    uint8_t begin();
    uint8_t end();
	void portHandler();
	void readStatus();
	void sendMCmd();
	uint32_t readComSpeed();
	void readPage();
	void writePage();
	uint8_t readISDU(vector<uint8_t>& oData, uint16_t index, uint8_t subIndex);
	uint8_t writeISDU(uint8_t sizeData, vector<uint8_t>& oData, uint16_t index, uint8_t subIndex);
	uint8_t readDirectParameterPage(uint8_t address, uint8_t *pData);
	uint8_t readPD(vector<uint8_t>& pData);
	uint8_t writePD(uint8_t sizeData, uint8_t *pData, uint8_t sizeAnswer);
	void readDI();
	void readCQ();
	void writeCQ();
	void isDeviceConnected();
    tuple<uint16_t, uint32_t> getDeviceId();
    tuple<uint8_t, uint8_t, uint8_t> getLengthParameter();
    uint8_t readErrorRegister();
    PDclass* get_PDclass();
    vector<uint8_t> get_lastIsduRequest();
    bool get_DeviceConnection();
};

#endif //IOLMASTERPORTMAX14819_H_INCLUDED
