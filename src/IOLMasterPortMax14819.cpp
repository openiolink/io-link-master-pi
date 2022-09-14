/*!
 * @file IOLMasterPortMax14819.cpp
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

//!***** Header-Files ************************************************************
#include "IOLMasterPortMax14819.h"
#include "Max14819.h"
#include "IOLink.h"
#include <iostream>
#include <tuple>
#include <algorithm> //for reverse-function
#include <string>
#include <cstring>
#include <math.h>
#include "IoddManager.h"
#include <cstdio>

//!***** Implementation **********************************************************

//!*******************************************************************************
//!  function :    IOLMasterPortMax14819
//!*******************************************************************************
//!  \brief        Constructor for IOLMasterPortMax14819
//!
//!  \type         local
//!
//!  \param[in]    void
//!
//!  \return       void
//!
//!*******************************************************************************
IOLMasterPortMax14819::IOLMasterPortMax14819()
    : pDriver_(nullptr),
      port_(max14819::PORTA),
      portType_(0),
      diModeSupport_(0),
      portMode_(0),
      portStatus_(0),
      actualCycleTime_(0),
      comSpeed_(0)
{
}

//!*******************************************************************************
//!  function :    IOLMasterPortMax14819
//!*******************************************************************************
//!  \brief        Constructor for IOLMasterPortMax14819
//!
//!  \type         local
//!
//!  \param[in]    void
//!
//!  \return       void
//!
//!*******************************************************************************
IOLMasterPortMax14819::IOLMasterPortMax14819(max14819::Max14819 *pDriver, max14819::PortSelect port)
    : pDriver_(pDriver),
      port_(port),
      portType_(0),
      diModeSupport_(0),
      portMode_(0),
      portStatus_(0),
      actualCycleTime_(0),
      comSpeed_(0),
      VendorID_(0),
      DeviceID_(0),
      mSequenceType_(0),
      OnRequestData_(0),
      ProcessDataIn_(0),
      ProcessDataOut_(0),
      ProcessDataInByte_(0),
      ProcessDataOutByte_(0)
{
}
//!*******************************************************************************
//!  function :    ~IOLMasterPort
//!*******************************************************************************
//!  \brief        Destructor for IOLMasterPort
//!
//!  \type         local
//!
//!  \param[in]    void
//!
//!  \return       void
//!
//!*******************************************************************************
IOLMasterPortMax14819::~IOLMasterPortMax14819()
{
}
//!*******************************************************************************
//!  function :    begin
//!*******************************************************************************
//!  \brief        Initialize port and connect to io-link device if attached.
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       0 if success
//!
//!*******************************************************************************
uint8_t IOLMasterPortMax14819::begin()
{
    char buf[256];
    uint8_t retValue = SUCCESS;

    // Initialize drivers
    if (pDriver_->begin(port_) == ERROR)
    {
        retValue = ERROR;
        pDriver_->Serial_Write("Error initialize driver01 PortA");
    }

    pDriver_->Serial_Write("WakeUp");
    // Generate wakeup
    retValue = uint8_t(retValue | pDriver_->wakeUpRequest(port_, &comSpeed_)); //comSpeed as a pointer gets value in the function wakeUpRequest
    if (retValue == ERROR)
    {
        pDriver_->Serial_Write("Error wakeup driver01 PortA");
        deviceConnection=1;
    }
    else
    {
        deviceConnection=0;
        sprintf(buf, "Communication established with %d bauds\n", comSpeed_); // TODO:
        pDriver_->Serial_Write(buf);
        // TODO: Serial.print("Communication established with ");
        // TODO: Serial.print(comSpeed_);
        // TODO: Serial.print(" Baud/s \n");
        // BOS DEBUG
//        pDriver_->wait_for(100);
        // BOS DEBUG

        pDriver_->Serial_Write("Device");
        uint8_t pData[3];
        // M-sequence Capability (IOL-Specification page: 239)
        readDirectParameterPage(IOL::PAGE::M_SEQ_CAP, pData);
        mSequenceType_ = uint8_t((pData[0] >> 1) & 0x07); // shift 1 to the right (first bit is ISDU support bit), clear all bits except first three (get a range of possible values: 0 - 7)
        //cout<<"MSequence Type: "<<int(mSequenceType_)<<endl;
        //RevisionID IOL-Version
        readDirectParameterPage(IOL::PAGE::REVISION_ID, pData);
        RevisionID_ = uint8_t(pData[0]);
        // ProcessDataIn
        readDirectParameterPage(IOL::PAGE::PD_IN, pData);
        ProcessDataIn_ = uint8_t(pData[0] & 0x1F); // get pData in Range: 0 - 31 (5 Bits)
        ProcessDataInByte_ = uint8_t((pData[0] >> 7) & 1); // read last bit of the Byte
        //cout<<"ProcessDataIn_: "<<int(ProcessDataIn_)<<endl;
        //cout<<"ProcessDataInByte_: "<<int(ProcessDataInByte_)<<endl;
        
        // ProcessDataOut
        readDirectParameterPage(IOL::PAGE::PD_OUT, pData);
        ProcessDataOut_ = uint8_t(pData[0] & 0x1F); // get pData in Range: 0 - 7
        ProcessDataOutByte_ = uint8_t((pData[0] >> 7) & 1); // read last bit of the Byte
        //cout<<"ProcessDataOut_: "<<int(ProcessDataOut_)<<endl;
        //cout<<"ProcessDataOutByte_: "<<int(ProcessDataOutByte_)<<endl;

        //PDin/out OD length calculation==================================================
        uint8_t ProcessDataInLength=0;
        uint8_t ProcessDataOutLength=0;
        //======FIRST TABLE==== IOL-Spec (page 240, Table B.6)
        //=========PDIN (first table)========
        if(ProcessDataInByte_) 
        {
            switch(ProcessDataIn_)
            {
                case 0||1:
                cout<<"ERROR - Reserved Length of ProcessDataIn_"<<endl;
                break;
                case 2 ... 31:
                ProcessDataInLength = ProcessDataIn_+1;
                break;
                default:
                cout<<"ERROR - Length of ProcessDataIn_ out of range_1"<<endl;
            }
        }
        else
        {
            switch(ProcessDataIn_)
            {
                case 0:
                cout<<"ERROR - Reserved Length of ProcessDataIn_"<<endl;
                break;
                case 1 ... 8:
                ProcessDataInLength=1;
                break;
                case 9 ... 16:
                ProcessDataInLength=2;
                break;
                default:
                cout<<"ERROR - Length of ProcessDataIn_ out of range_2"<<endl;
            }

        }

        //========PDOUT (first table )======== (IOL-Specification, page 240, Table B.6)

        if(ProcessDataOutByte_)
        {
            switch(ProcessDataOut_)
            {
                case 0||1:
                cout<<"ERROR - Reserved Length of ProcessDataIn_"<<endl;
                break;
                case 2 ... 31:
                ProcessDataOutLength = ProcessDataOut_+1;
                break;
                default:
                cout<<"ERROR - Length of ProcessDataIn_ out of range_3"<<endl;
            }
        }
        else
        {
            switch(ProcessDataOut_)
            {
                case 0:
                break;
                case 1 ... 8:
                ProcessDataOutLength=1;
                break;
                case 9 ...16:
                ProcessDataOutLength=2;
                break;
                default:
                cout<<"ERROR - Length of ProcessDataIn_ out of range_4"<<endl;
            }

        }

        //========SECOND TABLE======== (IOL Specification, page 225, Table A.10)
        if((ProcessDataInLength==0)&&(ProcessDataOutLength==0))
        {
            switch(mSequenceType_)
            {
                case 0:
                OnRequestData_=1;
                mSequenceType_=IOL::M_TYPE_0;
                break;
                case 1:
                OnRequestData_=2;
                mSequenceType_=IOL::M_TYPE_1_X; //TYPE_1_2
                break;
                case 6:
                OnRequestData_=8;
                mSequenceType_=IOL::M_TYPE_1_X; //TYPE_1_V
                break;
                case 7:
                OnRequestData_=32;
                mSequenceType_=IOL::M_TYPE_1_X; //TYPE_1_V
                break;
                default:
                cout<< "ERROR - no matching M-Sequence Type - Length"<<endl;
            }
        }
        else if((ProcessDataInLength==1)&&(ProcessDataOutLength==0)&&(mSequenceType_==0))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_1
        }
        else if((ProcessDataInLength==2)&&(ProcessDataOutLength==0)&&(mSequenceType_==0))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_2
        }
        else if((ProcessDataInLength==0)&&(ProcessDataOutLength==1)&&(mSequenceType_==0))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_3
        }
        else if((ProcessDataInLength==0)&&(ProcessDataOutLength==2)&&(mSequenceType_==0))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_4
        }
        else if((ProcessDataInLength==1)&&(ProcessDataOutLength==1)&&(mSequenceType_==0))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_5
        }
        else if((ProcessDataInLength==2)&&(ProcessDataOutLength==(1||2))&&(mSequenceType_==0))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength==(1||2))&&(ProcessDataOutLength==2)&&(mSequenceType_==0))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        //SECOND HALF OF TABLE==
        else if((ProcessDataInLength>=0)&&(ProcessDataOutLength>=3)&&(mSequenceType_==4))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength>=3)&&(ProcessDataOutLength>=0)&&(mSequenceType_==4))
        {
            OnRequestData_=1;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength>0)&&(ProcessDataOutLength>=0)&&(mSequenceType_==5))
        {
            OnRequestData_=2;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength>=0)&&(ProcessDataOutLength>0)&&(mSequenceType_==5))
        {
            OnRequestData_=2;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength>0)&&(ProcessDataOutLength>=0)&&(mSequenceType_==6))
        {
            OnRequestData_=8;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength>=0)&&(ProcessDataOutLength>0)&&(mSequenceType_==6))
        {
            OnRequestData_=8;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength>0)&&(ProcessDataOutLength>=0)&&(mSequenceType_==7))
        {
            OnRequestData_=32;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        else if((ProcessDataInLength>=0)&&(ProcessDataOutLength>0)&&(mSequenceType_==7))
        {
            OnRequestData_=32;
            mSequenceType_=IOL::M_TYPE_2_X; //TYPE_2_V
        }
        //Overwrite calculated values
        ProcessDataIn_=ProcessDataInLength;
        ProcessDataOut_=ProcessDataOutLength;
        //End of Calculation=============================================

        // VendorID (writeen in string)
        readDirectParameterPage(IOL::PAGE::VENDOR_ID1, pData);     //MSB = Most Significant Bit
        readDirectParameterPage(IOL::PAGE::VENDOR_ID2, pData + 1); //LSB = Least Significant Bit
        VendorID_ = uint16_t((pData[0] << 8) | pData[1]);
        // DeviceID
        readDirectParameterPage(IOL::PAGE::DEVICE_ID1, pData); //MSB
        readDirectParameterPage(IOL::PAGE::DEVICE_ID2, pData + 1);
        readDirectParameterPage(IOL::PAGE::DEVICE_ID3, pData + 2); //LSB
        DeviceID_ = (pData[0] << 16) | (pData[1] << 8) | pData[2];

        //quick fix BES (OD Data = 2 Byte anstatt 1 Byte)
        if(DeviceID_==132099)
        {
            OnRequestData_=2;
        }

        sprintf(buf, "Vendor ID: %d, Device ID: %d, MSequenceType: %d, ProcessDataIn: %d, ProcessDataOut: %d, OD: %d, RevisionID: %d\n", VendorID_, DeviceID_, mSequenceType_, ProcessDataIn_, ProcessDataOut_, OnRequestData_, RevisionID_);
        pDriver_->Serial_Write(buf);
    

    if (DeviceID_ == 263955) { //TODO: BCM timing problem, check if necessary
    pDriver_->wait_for(1000);
    }
    //std::string ioddRev("1.1");
    //std::shared_ptr<std::string> parsedIODD = iodd::IoddStore::getInstance().getIoddFile(VendorID_, DeviceID_, ioddRev);
    //std::shared_ptr<std::string> parsedIODD = iodd::IoddStore::getInstance().getIoddFile(VendorID_, uint32_t(917761), ioddRev);
    //cout << *parsedIODD << endl;

    //for (auto it=parsedIODD.begin(); it != parsedIODD.end(); ++it)
    //{
    //    cout << ' ' << *it;  
    //}
    
    //cout << parsedIODD << endl;

    // Switch from STARTUP Mode directly (without PREOPERATE) to OPERATE Mode (IOL-Spec page 75)
    uint8_t value[1] = {IOL::MC::DEV_OPERATE};
    if (pDriver_->writeData(IOL::MC::PAGE_WRITE, 1, value, 1, IOL::M_TYPE_0, port_) == ERROR)
    {
        sprintf(buf, "Error operate driver01 PortA"); // TODO:
        pDriver_->Serial_Write(buf);
    }
    if(ProcessDataOut_)
    {
        //ProzessData initial mit 0 Beschreiben
        vector<uint8_t>pDataOut;
        for(uint8_t i=0; i<ProcessDataOut_;i++)
        {
            pDataOut.push_back(0);
        }
        get_PDclass()->write_procDataOut(pDataOut);

        //MC für valide PDout Daten senden
        pDriver_->wait_for(200);
        uint8_t value2[ProcessDataOut_+OnRequestData_];
        for(int i=0;i<ProcessDataOut_+OnRequestData_;i++)
        {
            if(i==(ProcessDataOut_))
            {
                value2[i]=IOL::MC::PDOUT_VALID; //place MC on first Byte of OD Data
            }
            else
            {
                value2[i]=0;
            }
        }
        cout<<"PDOUT erforderlicher Mastercommand wurde gesendet"<<endl;
        
        pDriver_->writeData(IOL::MC::PAGE_WRITE,ProcessDataOut_+OnRequestData_,value2,1,mSequenceType_,port_);
    
        //quick fix BOS0285
        //first message doesn't send the right bits (parity error or something else is the fault)
        if(DeviceID_==264968)
        {
            vector<uint8_t> oData;
            for(uint8_t i=0;i<2;i++)
            {
                pDriver_->wait_for(10);
                readISDU(oData,0x0010,0x00);
                oData.clear();
            }
        }
    }
    get_PDclass()->set_iodd(VendorID_, DeviceID_, RevisionID_);
    //pDriver_->wait_for(200);
    }
    return retValue;
}

//!*******************************************************************************
//!  function :    end
//!*******************************************************************************
//!  \brief        Disconnect from device an reset port.
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       0 if success
//!
//!*******************************************************************************
uint8_t IOLMasterPortMax14819::end()
{
    cout << "VendorID: " << VendorID_ << " DeviceID: " << DeviceID_ << endl;
    uint8_t retValue = SUCCESS;

    pDriver_->Serial_Write("Shutdown");

    // Send device fallback command
    retValue = uint8_t(retValue | pDriver_->writeData(IOL::MC::DEV_FALLBACK, 0, nullptr, 1, IOL::M_TYPE_0, port_));

    // Reset port
    retValue = uint8_t(retValue | pDriver_->reset(port_));

    return retValue;
}

//!*******************************************************************************
//!  function :    readPD
//!*******************************************************************************
//!  \brief        Sends a process data request to the device and receive the
//!                answer from the slave.
//!
//!  \type         local
//!
//!  \param[in]    &pData               reference of pData
//!
//!  \return       0 if success and processdata valid
//!
//!*******************************************************************************

uint8_t IOLMasterPortMax14819::readPD(vector<uint8_t>& pData) //wird aktuell verwendet
{
    uint8_t retValue = SUCCESS;
    uint8_t sizeAnswer = ProcessDataIn_ + OnRequestData_;

    if (ProcessDataOut_ > 0)
    {
        vector<uint8_t> pOut;
        for(int i=0; i<ProcessDataOut_;i++)
        {
            pOut.push_back(pdclass.get_procDataOut().at(i));
        }
        uint8_t *ppOut=&pOut[0];
        // Send process data request to device
        retValue = uint8_t(retValue | pDriver_->writeData(IOL::MC::PD_READ, uint8_t(ProcessDataOut_), ppOut, sizeAnswer, mSequenceType_, port_));
        pOut.clear();
    }
    else
    {
        // Send process data request to device
        retValue = uint8_t(retValue | pDriver_->writeData(IOL::MC::PD_READ, 0, nullptr, sizeAnswer, mSequenceType_, port_));
    }
    pDriver_->wait_for(5);
    // read received answer
    retValue = uint8_t(retValue | pDriver_->readPD(pData, sizeAnswer, port_, OnRequestData_));
    deviceConnection=retValue;
    return retValue;
}
//!*******************************************************************************
//!  function :    writePD
//!*******************************************************************************
//!  \brief        Sends process data to the device. Information's like length and
//!                m-sequence type must be set by user.
//!
//!  \type         local
//!
//!  \param[in]    sizeData             size in Byte of data
//!  \param[in]    *pData               pointer to data
//!  \param[in]    sizeAnswer           size in byte of answer
//!  \param[in]    mSeqType             M-sequence type
//!
//!  \return       0 if success
//!
//!*******************************************************************************
uint8_t IOLMasterPortMax14819::writePD(uint8_t sizeData, uint8_t *pData, uint8_t sizeAnswer)
{
    uint8_t retValue = SUCCESS;
    pDriver_->wait_for(10);
    /*for(int i=0;i<sizeData;i++)
    {
        cout<<"Daten: "<<int(pData[i])<<endl;
    }*/
    retValue = uint8_t(retValue | pDriver_->writeData(IOL::MC::PAGE_WRITE, ProcessDataOut_+OnRequestData_, pData, sizeAnswer, mSequenceType_, port_)); //Write Data with PAGE_WRITE MC because of PDValid
    return retValue;
}
//!*******************************************************************************
//!  function :    readISDU
//!*******************************************************************************
//!  \brief        The readISDU service is used to write On-request Data to a
//!                Device connected to a specific port
//!
//!  \type         local
//!
//!  \param[in]     *data               pointer to transmit data
//!  \param[in]     index	            index of register
//!  \param[in]     subIndex            subindex of register
//!
//!  \return        0 if success
//!
//!*******************************************************************************
uint8_t IOLMasterPortMax14819::readISDU(vector<uint8_t>& oData, uint16_t index, uint8_t subIndex)
{
    uint8_t sizeAnswer=32;
    uint8_t retValue = SUCCESS;
    uint8_t iService = 0;
    uint8_t highIndex = (index & 0xFF00) >> 8;
    uint8_t lowIndex = index & 0x00FF;
    vector<uint8_t> isduDataFrame;
    if (index < 256)
    {
        if (subIndex == 0) //subindex 0 is used to reference the entire data object
        {
            //cout << "8 Bit" << endl;
            iService = uint8_t((IOL::ISDU::READ_REQ_8BIT << 4) + 0x3u);
            //cout<<"\nvar 1 iService: "<<int(iService)<<endl;
            isduDataFrame.push_back(iService);
            isduDataFrame.push_back(lowIndex);
        }
        else
        {
            //cout << "8 Bit + Subindex" << endl;
            iService = uint8_t((IOL::ISDU::READ_REQ_8BIT_SUB << 4) + 0x4u);
            //cout<<"\nvar 2 iService: "<<int(iService)<<endl;
            isduDataFrame.push_back(iService);
            isduDataFrame.push_back(lowIndex);
            isduDataFrame.push_back(subIndex);
        }
    }
    else
    {
        //cout << "16 Bit + Subindex" << endl;
        iService = uint8_t((IOL::ISDU::READ_REQ_16BIT << 4) + 0x5u);
        //cout<<"\nvar 3 iService: "<<int(iService)<<endl;
        isduDataFrame.push_back(iService);
        isduDataFrame.push_back(highIndex);
        isduDataFrame.push_back(lowIndex);
        isduDataFrame.push_back(subIndex);
    }
    //Calculate Checksum
    uint8_t chkpdu = pDriver_->calculateCHKPDU(isduDataFrame);
    isduDataFrame.push_back(chkpdu);
    //cout<<"chkpdu: "<<int(chkpdu)<<endl;
    int8_t byte2send = OnRequestData_ - isduDataFrame.size();//-ProcessDataOut_;
    if (byte2send<0) byte2send=0;
        //cout<<"byte2send: "<<int(byte2send)<<endl;
    for (uint8_t i = 0; i < byte2send; i++)
    {
        isduDataFrame.push_back(0x00u);
        //cout<<"bufferauffüllen: 0x00"<<endl;
    }

    vector<uint8_t> pdout;
    pdout=get_PDclass()->get_procDataOut();
    uint8_t *pdoutptr=&pdout[0];

    //==== Send ISDU - Request to device -> ISDU Answer from device ====
    
    uint8_t zaehlvar=0;
    uint8_t timeout=0;
    vector<uint8_t> vectortosend;
    //cout<<"isduDataFramesize(): "<<int(isduDataFrame.size())<<endl;

        //Send request to device
        while(((zaehlvar)*OnRequestData_)<(isduDataFrame.size()))//Send Request to device
        {
            //cout<<"WhileSchleife"<<endl;
            vectortosend.clear();
            for(uint8_t i=0;i<ProcessDataOut_;i++)
            {
                //cout<<"PDOUT!!!!"<<endl;
                vectortosend.push_back(uint8_t(0x00));
            }
            for(uint8_t i=0;i<OnRequestData_;i++)
            {
                //cout<<"Forschleife"<<endl;
                vectortosend.push_back(isduDataFrame[((zaehlvar*OnRequestData_)+i)]); //Send as much Data as the device can send
                //cout<<"vectorzugriff erfolgreich"<<endl;
            }
            if(zaehlvar==0)
            {
                //cout<<"ifbedingung"<<endl;
                //cout<<"vectortosend.size(): "<<int(vectortosend.size())<<endl;
                retValue = uint8_t(retValue | pDriver_->writeISDU(IOL::MC::OD_WRITE,0, mSequenceType_, port_, vectortosend, ProcessDataOut_, isduDataFrame));
                pDriver_->wait_for(5);
            }
            else
            {
                //cout<<"elseBedingung"<<endl;
                //cout<<"vectortosend.size(): "<<int(vectortosend.size())<<endl;
                retValue = uint8_t(retValue | pDriver_->writeISDU(uint8_t(IOL::MC::OD_FLOWCTRL+zaehlvar),0, mSequenceType_, port_, vectortosend, ProcessDataOut_, isduDataFrame));
                pDriver_->wait_for(5);
            }
            vectortosend.clear();
            if(zaehlvar==15)
            {
                if(timeout>=3)return retValue=ERROR; //Timeout löst aus
                timeout++;
                zaehlvar=0;
            }
            else
            {
                zaehlvar++;
            }
        }
        timeout=0;
        //cout<<"try to receive Data"<<endl;
        //Receive data from device
        do
        {
            //cout<<"in der whileschleife"<<endl;
            oData.clear();

            if(ProcessDataOut_)
            {
                retValue = uint8_t(retValue | pDriver_->writeData(IOL::MC::OD_READ, pdout.size(), pdoutptr, sizeAnswer, mSequenceType_, port_));
            }
            else
            {
                retValue = uint8_t(retValue | pDriver_->writeData(IOL::MC::OD_READ, 0, nullptr, sizeAnswer, mSequenceType_, port_));
            }
            pDriver_->wait_for(5);

            retValue = uint8_t(retValue | pDriver_->readISDU(oData, OnRequestData_, port_));

            timeout++;
            //cout<<int(timeout)<<endl;
            if(timeout>=254) return retValue=ERROR; //timeout, if device doesn't respond 0 or 1

        }while(oData[0]==1||oData[0]==0);

        sizeAnswer = uint8_t(oData[0]&0xF);
        
        int loops=sizeAnswer/OnRequestData_;
        if(loops<0)loops=0;
        //cout<<"loops: "<<int(loops)<<endl;
        for(int i=0;i<loops;i++)
        {
            if(ProcessDataOut_)
            {
                retValue = uint8_t(retValue | pDriver_->writeData((225+i), pdout.size(), pdoutptr, sizeAnswer, mSequenceType_, port_));
            }
            else
            {
                retValue = uint8_t(retValue | pDriver_->writeData((225+i), 0, nullptr, sizeAnswer, mSequenceType_, port_));
            }
            pDriver_->wait_for(15);
            retValue = uint8_t(retValue | pDriver_->readISDU(oData, uint8_t(OnRequestData_), port_));
        }

        //vector oData in Format: (iService+length) (Data in Bytes....) (Checksum)

        oData.erase(oData.begin()+sizeAnswer-1); //erase iService+length
        oData.erase(oData.begin()); //erase Checksum

        //vector oData in Format: (Data in Bytes.....)

        return retValue;

    //}
}

//!*******************************************************************************
//!  function :    writeISDU
//!*******************************************************************************
//!  \brief        The AL_Write service is used to write On-request Data to a
//!                Device connected to a specific port
//!
//!  \type         local
//!
//!  \param[in]     uint8_t mc			master command
//!  \param[in]     sizeData	        size in Byte of data
//!  \param[in]     *data               pointer to transmit data
//!  \param[in]     sizeAnswer	        size in byte of answer
//!  \param[in]     mSeqType            m-sequence type depending
//!  \param[in]     port		        port to send data
//!
//!  \return        0 if success
//!
//!*******************************************************************************
uint8_t IOLMasterPortMax14819::writeISDU(uint8_t sizeData, vector<uint8_t>& oData, uint16_t index, uint8_t subIndex)
{
    uint8_t sizeAnswer=ProcessDataIn_; //CKS
    uint8_t retValue = SUCCESS;
    uint8_t iService = 0;
    uint8_t highIndex = (index & 0xFF00) >> 8;
    uint8_t lowIndex = index & 0x00FF;
    vector<uint8_t> isduDataFrame;
    if (index < 256)
    {
        if (subIndex == 0) //subindex 0 is used to reference the entire data object
        {
            //sizeAnswer=2;
            //cout << "8 Bit" << endl;
            iService = uint8_t((IOL::ISDU::WRITE_REQ_8BIT << 4) + sizeData + 0x3u);
            //cout<<"\nvar 1 iService: "<<int(iService)<<endl;
            isduDataFrame.push_back(iService);
            isduDataFrame.push_back(lowIndex);
        }
        else
        {
            //sizeAnswer=4;
            //cout << "8 Bit + Subindex" << endl;
            iService = uint8_t((IOL::ISDU::WRITE_REQ_8BIT_SUB << 4) + sizeData + 0x4u);
            //cout<<"\nvar 2 iService: "<<int(iService)<<endl;
            isduDataFrame.push_back(iService);
            isduDataFrame.push_back(lowIndex);
            isduDataFrame.push_back(subIndex);
        }
    }
    else
    {
        //cout << "16 Bit + Subindex" << endl;
        iService = uint8_t((IOL::ISDU::WRITE_REQ_16BIT << 4) + sizeData + 0x5u);
        //cout<<"\nvar 3 iService: "<<int(iService)<<endl;
        isduDataFrame.push_back(iService);
        isduDataFrame.push_back(highIndex);
        isduDataFrame.push_back(lowIndex);
        isduDataFrame.push_back(subIndex);
    }
    
    //get Data in the isduDataFrame
    for(int i=0;i<oData.size();i++)
    {
        isduDataFrame.push_back(oData[i]);
        //cout<<"oData: "<<int(oData[i])<<endl;
    }
    //Calculate Checksum
    uint8_t chkpdu = pDriver_->calculateCHKPDU(isduDataFrame);
    isduDataFrame.push_back(chkpdu);
    //cout<<"chkpdu: "<<int(chkpdu)<<endl;
    int8_t byte2send = OnRequestData_ - isduDataFrame.size();//-ProcessDataOut_;
    if (byte2send<0) byte2send=0;
        //cout<<"byte2send: "<<int(byte2send)<<endl;
    for (uint8_t i = 0; i < byte2send; i++)
    {
        isduDataFrame.push_back(0x00u);
        //cout<<"bufferauffüllen: 0x00"<<endl;
    }

    vector<uint8_t> pdout;    
    pdout=get_PDclass()->get_procDataOut();
    uint8_t *pdoutptr=&pdout[0];


    //==== Send ISDU - Request to device -> ISDU Answer from device ====
    
    uint8_t zaehlvar=0;
    uint8_t timeout=0;
    vector<uint8_t> vectortosend;
    //cout<<"isduDataFramesize(): "<<int(isduDataFrame.size())<<endl;

        //Send request to device
        while(((zaehlvar)*OnRequestData_)<(isduDataFrame.size()))//Send Request to device
        {
            //cout<<"WhileSchleife"<<endl;
            vectortosend.clear();
            for(uint8_t i=0;i<ProcessDataOut_;i++)
            {
                //cout<<"PDOUT!!!!"<<endl;
                vectortosend.push_back(uint8_t(0x00));
            }
            for(uint8_t i=0;i<OnRequestData_;i++)
            {
                //cout<<"Forschleife"<<endl;
                vectortosend.push_back(isduDataFrame[((zaehlvar*OnRequestData_)+i)]); //Send as much Data as the device can send
                //cout<<"vectorzugriff erfolgreich"<<endl;
            }
            if(zaehlvar==0)
            {
                //cout<<"ifbedingung"<<endl;
                //cout<<"vectortosend.size(): "<<int(vectortosend.size())<<endl;
                retValue = uint8_t(retValue | pDriver_->writeISDU(IOL::MC::OD_WRITE,0, mSequenceType_, port_, vectortosend, ProcessDataOut_, isduDataFrame));
                pDriver_->wait_for(5);
            }
            else
            {
                //cout<<"elseBedingung"<<endl;
                //cout<<"vectortosend.size(): "<<int(vectortosend.size())<<endl;
                retValue = uint8_t(retValue | pDriver_->writeISDU(uint8_t(IOL::MC::OD_FLOWCTRL+zaehlvar),0, mSequenceType_, port_, vectortosend, ProcessDataOut_, isduDataFrame));
                pDriver_->wait_for(5);
            }
            vectortosend.clear();
            if(zaehlvar==15)
            {
                if(timeout>=3)return retValue=ERROR; //Timeout löst aus
                timeout++;
                zaehlvar=0;
            }
            else
            {
                zaehlvar++;
            }
        }
        timeout=0;
    return retValue;
}

//!*******************************************************************************
//!  function :    readDirectParameterPage
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
uint8_t IOLMasterPortMax14819::readDirectParameterPage(uint8_t address, uint8_t *pData)
{
    if (address > 31)
    {
        pDriver_->Serial_Write("readDirectParameterPage: address to big\n");
        return 0;
    }
    uint8_t retValue = SUCCESS;

    // Send processdata request to device
    retValue = uint8_t(retValue | pDriver_->writeData((IOL::MC::PAGE_READ + address), 0, nullptr, 1, IOL::M_TYPE_0, port_));

    pDriver_->wait_for(10);

    // Receive answer
    retValue = uint8_t(retValue | pDriver_->readData(pData, 1, port_));

    return retValue;
}

//!*******************************************************************************
//!  function :    portHandler
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::portHandler()
{
}

//!*******************************************************************************
//!  function :    readStatus
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::readStatus()
{
}

//!*******************************************************************************
//!  function :    sendMCmd
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::sendMCmd()
{
}

//!*******************************************************************************
//!  function :    readComSpeed
//!*******************************************************************************
//!  \brief        Returns the communication speed of the port.
//!
//!  \type         local
//!
//!  \param[in]    void
//!
//!  \return       communication speed
//!
//!*******************************************************************************
uint32_t IOLMasterPortMax14819::readComSpeed()
{
    return comSpeed_;
}

//!*******************************************************************************
//!  function :    readPage
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::readPage()
{
}

//!*******************************************************************************
//!  function :    writePage
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::writePage()
{
}

//!*******************************************************************************
//!  function :    readISDU
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
//void IOLMasterPortMax14819::readISDU() {
//
//}

//!*******************************************************************************
//!  function :    writeISDU
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
//void IOLMasterPortMax14819::writeISDU() {
//
//}

//!*******************************************************************************
//!  function :    readDI
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::readDI()
{
}

//!*******************************************************************************
//!  function :    readCQ
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::readCQ()
{
}

//!*******************************************************************************
//!  function :    writeCQ
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::writeCQ()
{
}

//!*******************************************************************************
//!  function :    isDeviceConnected
//!*******************************************************************************
//!  \brief        Not implemented yet
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************
void IOLMasterPortMax14819::isDeviceConnected()
{
    uint8_t retVal = SUCCESS;
    uint8_t pData[3];
    uint16_t newVendorID=0;
    uint32_t newDeviceID=0;
    //cout<<"BEFORE if-Schleife:"<<endl;
    //check if there has been a device connected
    if(deviceConnection==0)
    {
        //cout<<"Device connected"<<endl;
    }
    else
    {
        //cout<<"Checking DeviceConnection..."<<endl;
        retVal=begin(); //initialize port
    }
    return;
}

tuple<uint16_t, uint32_t> IOLMasterPortMax14819::getDeviceId()
{
    return make_tuple(VendorID_, DeviceID_);
}

tuple<uint8_t, uint8_t, uint8_t> IOLMasterPortMax14819::getLengthParameter()
{
    return make_tuple(OnRequestData_, ProcessDataIn_, ProcessDataOut_);
}

//!*******************************************************************************
//!  function :    get_PD
//!*******************************************************************************
//!  \brief        get the union Parameters adress
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       union ProcessDataIn adress
//!
//!*******************************************************************************


uint8_t IOLMasterPortMax14819::readErrorRegister()
{
    uint8_t retValue=0;
//        retValue = uint8_t(retValue | pDriver_->writeRegister(0x09u, uint8_t(0x01u | 230400)));
//        cout << "ISDU ErrorregisterA: "<< int(readRegister(CQErrA)) << endl;
//        Hardware->wait_for(5);
//        retValue = (uint8_t)(retValue | pDriver_->writeData(IOL::MC::OD_READ, 0, nullptr, 1, 2, port_)); //trigger "busy"
//        Hardware->wait_for(5);
//        cout << "ISDU ErrorregisterA: "<< int(readRegister(CQErrA)) << endl;
//        retValue = (uint8_t)(retValue | writeData(IOL::MC::OD_READ, 0, nullptr, sizeAnswer, mSeqType, port));
retValue = pDriver_->readRegister(0x08);
return retValue;
}
    PDclass* IOLMasterPortMax14819::get_PDclass()
    {
        return &pdclass;
    }
//uint8_t IOLMasterPortMax14819::readCondition()
//{

//}
//===================================PDclass========================================================

//!*******************************************************************************
//!  function :    PDclass
//!*******************************************************************************
//!  \brief        Constructor for PDclass
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************

    PDclass::PDclass()
    {

    }

//!*******************************************************************************
//!  function :    ~PDclass
//!*******************************************************************************
//!  \brief        Destruktor for PDclass
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       void
//!
//!*******************************************************************************

    PDclass::~PDclass()
    {

    }

//!*******************************************************************************
//!  function :    write_pd_storage
//!*******************************************************************************
//!  \brief        write the demanded PD in its own variable
//!
//!  \type         local
//!
//!  \param[in]	   vector<uint8_t> PData
//!
//!  \return       void
//!
//!*******************************************************************************

    void PDclass::write_pd_storage(vector<uint8_t>PData)
    {
        procData=PData;

        return;
    }

//!*******************************************************************************
//!  function :    write_procDataOut
//!*******************************************************************************
//!  \brief        write process Data in storage variable which then can be sent
//!
//!  \type         local
//!
//!  \param[in]	   vector<uint8_t> PDout
//!
//!  \return       void
//!
//!*******************************************************************************

    void PDclass::write_procDataOut(vector<uint8_t>PDout)
    {
        procDataOut.clear();
        procDataOut=PDout;
        return;
    }

//!*******************************************************************************
//!  function :    get_float
//!*******************************************************************************
//!  \brief        get the float datatype out of the PDin Data, when requesting it
//!
//!  \type         local
//!
//!  \param[in]	   uint8_t length
//!
//!  \return       vector<float> pd_float
//!
//!*******************************************************************************

    vector<float> PDclass::get_float(uint8_t length)
    {
        vector<float> pd_float;
        uint32_t hilfs_value;
        float hilfs_float;
        for(int i=1;i<length;i=i+4) //first byte defines the length of the data
       {
            //order of pData: [] = b0, b1, b2, b3
            //order of newvalue: [] = b3, b2, b1, b0
            //order of memcpy: [] = b0, b1, b2, b3
            hilfs_value = ((procData[i+3]) | (procData[i+2] << 8) | (procData[i+1] << 16) | (procData[i] << 24)); //change Data Type (uint8_t -> float) and put Data in right order
            memcpy(&hilfs_float,&hilfs_value,sizeof(hilfs_float)); //writing the Data as float in the struct
            pd_float.push_back(hilfs_float);
        }
        return pd_float;
    }

//!*******************************************************************************
//!  function :    get_uint8_t
//!*******************************************************************************
//!  \brief        get the uint8_t datatype out of the PDin Data, when requesting it
//!
//!  \type         local
//!
//!  \param[in]	   uint8_t length
//!
//!  \return       vector<uint8_t> returnData
//!
//!*******************************************************************************

    vector<uint8_t> PDclass::get_uint8_t(uint8_t length)

    {
        vector<uint8_t> returnData=procData;

        reverse(returnData.begin(),returnData.end());

        returnData.erase(returnData.begin()+length);

        return returnData;
    }

//!*******************************************************************************
//!  function :    get_procDataOut
//!*******************************************************************************
//!  \brief        get PD Out out of the storage
//!
//!  \type         local
//!
//!  \param[in]	   uint8_t length
//!
//!  \return       vector<uint8_t> procDataOut
//!
//!*******************************************************************************

    vector<uint8_t> PDclass::get_procDataOut()
    {
        return procDataOut;
    }

//!*******************************************************************************
//!  function :    interpretProcessData()
//!*******************************************************************************
//!  \brief        get PD Out out of the storage in the right data format (IODD will tell)
//!
//!  \type         local
//!
//!  \param[in]	   void
//!
//!  \return       if SUCCESS -> nlohmann::json measurement
//!                if no SUCCESS -> nlohmann::json emptyjson
//!
//!*******************************************************************************

    nlohmann::json PDclass::interpretProcessData(IoddManager& instance)
    {
        vector<uint8_t> rawProcessData = procData;
        rawProcessData.erase(rawProcessData.begin()); //first byte defines the length of the data
        //cout << "Vendor ID: " << VendorID <<"  Device ID: " << DeviceID << " iolRev: " << iolRev <<"  ProcessDataSize: " << rawProcessData.size() << endl;
        std::tuple<nlohmann::json, nlohmann::json> transformedData = instance.interpretProcessData(rawProcessData, VendorID, DeviceID, iolRev);
        nlohmann::json measurement = std::get<0>(transformedData);
        nlohmann::json unitInfo = std::get<1>(transformedData);
        return measurement;
    }


//!*******************************************************************************
//!  function :    set_iodd
//!*******************************************************************************
//!  \brief        set the Variables VendorID, DeviceID, RevisionID in the PDClass
//!                for checking how to interpret the ProcessData
//!
//!  \type         local
//!
//!  \param[in]	   uint16_t VendorID_, uint32_t DeviceID_, uint8_t RevisionID_
//!
//!  \return       void
//!
//!*******************************************************************************

    void PDclass::set_iodd(uint16_t VendorID_, uint32_t DeviceID_, uint8_t RevisionID_)
    {
        VendorID =  VendorID_;
        DeviceID = DeviceID_;
        iolRev = RevisionID_;
    }

//!*******************************************************************************
//!  function :    set_iodd
//!*******************************************************************************
//!  \brief        quickly checking wether the port has a deviceConnection or not
//!
//!  \type         local
//!
//!  \param[in]	   uint8_t length
//!
//!  \return       bool deviceConnection
//!
//!*******************************************************************************

    bool IOLMasterPortMax14819::get_DeviceConnection()
    {
        return deviceConnection; //deviceConnection ==0 -> there is a device connected
    }