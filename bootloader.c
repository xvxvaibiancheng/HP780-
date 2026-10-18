/*******************************************************************************
Copyright (C), 1988-2008, HYT Tech. Co., Ltd.
FileName        : bootloader.c
Author          : 
Version         : 1.0
Date            : 2012.5.11
Description     : 
                : 
********************************************************************************
History: Scroll to the end of this file.
*******************************************************************************/

/*-------------------------- HEADER FILE INCLUDES ----------------------------*/


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f10x_conf.h"
//#include "arm_comm.h"
#include "mcuuart.h"
#include "McuTimer.h"
#include "apihrnp.h"
#include "bootloader.h"

/*----------------------------- ENUMERATIONS ---------------------------------*/

/*------------------------------- MACROS & CONSTANTS--------------------------*/
#define WAIT_CMD_TIME          500

/* version item */
#define BOOTLOADER_BASE_VERSION 0x00
#define BOOTLOADER_VERSION      0x01
#define APP_PROGRAM_VERSION     0x02

/*--------------------------- BASIC DATA TYPES -------------------------------*/
/* define mode for raw data transmit */
typedef enum
{
    NO_UPDATE           = 0x00,
    BOOTLOADER_UPDATA   = 0x01,
    PROGRAM_UPDATA      = 0x02,
    VERSION_READ        = 0x0A,
}EntryMode_t;

/*-------------------------- VAR DEFINITION ----------------------------*/
/* received raw data store buffer */
FrameBuf_t RawData = {0};
extern uint8_t Version[];

/*--------------------------------------------------------------------------------------
  Internal Function Prototype ( declaration of functions used in this file internally )
  Example: Static U8 xxx_xxx( void );
---------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------
  External Function Prototype ( declaration of functions which can be used externally )
  Example: U8 xxx_xxx( void );
---------------------------------------------------------------------------------------*/
void VersionReadProcess(void);
bool SendRawRqReply(void);
bool RunToAddress(uint32_t Addr);


/***************************************************************************
Function name   :RunToAddress
Description     :jump to appointed program address to run in flash area, this
                 function is used for CPU runing from one program to another, 
                 like from bootloader to APP program, bootloader and APP are 
                 two independcy program
Input           :new program start address 
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool RunToAddress(uint32_t Addr)
{
    typedef  void (*pFunction)(void);
    pFunction Jump_To_Application;
    uint32_t JumpAddress;
    bool result = FALSE;
    /* check the target program address area, whether in flash area or not */
    if(((*(volatile uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)    
    { 
        /* Jump to target address for a new application*/
        JumpAddress = *(volatile uint32_t*) (Addr + 4);
        Jump_To_Application = (pFunction) JumpAddress;
        /* Initialize the new application's Stack Pointer */
        __set_MSP(*(volatile uint32_t*)Addr);
        Jump_To_Application();
        result = TRUE;
    }
    return result;
}

/***************************************************************************
Function name   :RcvRawDataAndDecode
Description     :received raw data and decode it, the raw data is sent by flashburn
Input           :raw data store buffer, raw data mode type accept box 
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool RcvRawDataAndDecode(FrameBuf_t *pRawData, uint8_t *pRawDataCmdType)
{
    bool Result = FALSE;
    FrameBuf_t *GetHrnpCmd = &g_HrnpFrameScratch0;
    /* receive message */
    if(HrnpSlaveRcvCmdFromMaster(GetHrnpCmd))
    {
        uint16_t HdpepOpcode;
        uint16_t RawDataLength;
        if(DATA == GetHrnpCmd->RcvFrame[3])
        {
            /* get opcode and data length */
            if(RCPMODE == GetHrnpCmd->RcvFrame[12])
            {
                HdpepOpcode = ((GetHrnpCmd->RcvFrame[14] << 8) + GetHrnpCmd->RcvFrame[13]);
                RawDataLength = ((GetHrnpCmd->RcvFrame[18] << 8) + GetHrnpCmd->RcvFrame[17]);
            }
            else
            {
                HdpepOpcode = ((GetHrnpCmd->RcvFrame[13] << 8) + GetHrnpCmd->RcvFrame[14]);
                RawDataLength = ((GetHrnpCmd->RcvFrame[17] << 8) + GetHrnpCmd->RcvFrame[18]);
            }
            /* check whether the message is raw data or not*/
            if(RAW_DATA_TRANSMIT == HdpepOpcode)
            {
                /* get the message payload data mode, the mode is GOB defined used for reading version or other use
                     if need a new mode, just difined it in message[19], and send by flasburn or other PC tools */
                *pRawDataCmdType = GetHrnpCmd->RcvFrame[19];
                /* copy payload */
                pRawData->FrameLen = RawDataLength;
                memcpy(pRawData->RcvFrame, &(GetHrnpCmd->RcvFrame[19]), RawDataLength);
                Result = TRUE;
            } 
        }
    }
    return Result;
}

/***************************************************************************
Function name   :SendRawDataTransmitReply
Description     :reply the raw data reply message to master when received raw data
Input           :the responded result of received raw data.
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool SendRawDataTransmitReply(uint8_t Mode,uint8_t result)
{
	  SendResult_t Result = SEND_RESULT_CONTINUE;
    /* raw data reply message payload buffer:[1][2]size,[3]MODE,[4]RESULT */
    uint8_t ReplyPayload[4] = {0x02, 0x00, 0x00, 0x00};
    ReplyPayload[2] = Mode;
    ReplyPayload[3] = result;
    while(1)
    {
       
        Result = HrnpSlaveSendReplyToMaster(RCPMODE, RAW_DATA_TRANSMIT_REPLY, ReplyPayload, sizeof(ReplyPayload));
        if(SEND_RESULT_FAILED == Result)
        {
            return FALSE;
        }
        else if(SEND_RESULT_SUCCESSFUL == Result)
        {
            return TRUE;
        }
        else
        {}
    }
}

/***************************************************************************
Function name   :SendVersionRqReply
Description     :send the version message to master when received version read command
Input           :version buffer length, version buffer
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool SendVersionRqReply(uint8_t VersionItem, uint8_t *pVersion)
{
    uint8_t i, ReplyPayload[17] = {0x0F,0x00,0x00,0x00,0x00};
    SendResult_t Result ;

    ReplyPayload[2] = VERSION_READ;
    ReplyPayload[3] = SUCCESSFUL;
    ReplyPayload[4] = VersionItem;

    if(NULL == pVersion)
    {
        ReplyPayload[3] = FAILED;
    }
    else
    {
        /* get the version information to payload buffer */
        for(i = 0; i < 12; i++)
        {
            ReplyPayload[i+5] = pVersion[i];
        }
    }
    while(1)
    {
        Result = SEND_RESULT_CONTINUE;
        Result = HrnpSlaveSendReplyToMaster(RCPMODE, RAW_DATA_TRANSMIT_REPLY, ReplyPayload, sizeof(ReplyPayload));
        if(SEND_RESULT_FAILED == Result)
        {
            return FALSE;
        }
        else if(SEND_RESULT_SUCCESSFUL == Result)
        {
            return TRUE;
        }
        else
        {

        }
    }
}

/***************************************************************************
Function name   :SendRawRqReply
Description     :send notify message to flashburn via radio for telling flashburn 
                 that GOB program is at right place
Input           :none
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool SendRawRqReply(void)
{
    while(1)
    {
        SendResult_t Result = SEND_RESULT_CONTINUE;
        uint8_t ReplyPayload[] = {01, 00, 0xEE, 00};
        Result = HrnpSlaveSendReplyToMaster(RCPMODE, RAW_DATA_TRANSMIT_REPLY, ReplyPayload, sizeof(ReplyPayload));
        if(SEND_RESULT_FAILED == Result)
        {
            return FALSE;
        }
        else if(SEND_RESULT_SUCCESSFUL == Result)
        {
            return TRUE;
        }
        else
        {}
    }
}

/***************************************************************************
Function name   :VersionReadProcess
Description     :get version read command to read version
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
void VersionReadProcess(void)
{

}

