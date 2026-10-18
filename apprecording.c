/*******************************************************************************
Copyright (C), 1988-2008, HYT Tech. Co., Ltd.
FileName        : McuAd.cpp
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
//#include "arm_comm.h"
#include "mcuuart.h"

#include "mcutimer.h"
#include "mcugpio.h"
#include "apphdpep.h"
#include "bsp.h"
#include "bootloader.h"
#include "apprecording.h"
#include "apihrnp.h"
#include "delay.h"
#include "drv_wtn6.h"
#include "drv_lcd.h"
#include "eeprom.h"
#include "MX469.h"

//#include "Usrc_ADPCMCodec.h"

/*------------------------------------------------------------------------------
  Macro Define ( macro definitions used in this file internally )
------------------------------------------------------------------------------*/
#define RECDATAPAGEID       0xB7A9
#define RECDATALASTPAGEID   0xA5C7

/*------------------------------------------------------------------------------
  Structure Define ( globle variables definition used in this file internally )
------------------------------------------------------------------------------*/
VoiceRecord_t VoiceRecord       = {0};
RecDataSaveAddr_t RecDataAddrInfo;
RecDataSaveAddr_t PwrupPageSPAInfo;
uint16_t RxDataCnt = 0;
uint16_t RdDataCnt = 0;
uint8_t gRecMode = 0;
uint8_t gSpkEnableReq = 0;
uint8_t gSpkDisableReq = 0;
uint8_t gTxRecStartReq = 0;
uint8_t gTxRecStopReq = 0;
uint8_t gRxRecStartReq = 0;
uint8_t gRxRecStopReq = 0;
uint8_t PlayBackSwitch = 1;
uint8_t PlayNextAudioFlag = 0;
int16_t RadioVolumeValue = 0;
int16_t RadioVolumeValueTemp = 0;
uint16_t ChangeVolumeFlag = 0;

/*--------------------------------------------------------------------------------------
  Internal Function Prototype ( declaration of functions used in this file internally )
  Example: Static U8 xxx_xxx( void );
---------------------------------------------------------------------------------------*/
static bool AudioSpkClose(void);
static bool AudioSpkOpen(void);
bool AudioSpkConInit(void);
bool AudioSpkUnMute(u8 Enable);
bool AudioSpkMute(u8 Enable);
bool RadioSpkCon(uint8_t Enable);
bool Radio_Key_PTT_Congration(uint8_t ConStyle);
static void PlayAudioSpkSwitchProcess(void);
static void RegisterHdpepCmdRespondFunc(void);
bool RadioOBInit(void);
static void PowerUpBackUpRecordInfo(void);
static VoiceRecord_t GetTxBroadcastInfo(FrameBuf_t *pHrnpCmdMsg);
static VoiceRecord_t GetRxBroadcastInfo(FrameBuf_t *pHrnpCmdMsg);
static VoiceRecord_t GetStatusChangeInfo(FrameBuf_t *pHrnpCmdMsg);
bool RadioSendCommandFrame(uint32_t CommandOpCode,uint32_t CommandOpCodeREPLY,uint8_t *arrdata);
bool WindowBackLightCon(uint8_t setvalue);
static void ChangeRadioVolume(void);
static bool GetPostionData(void);
static bool GetRtcTimeData(void);
bool RadioPttCon(uint8_t PttEnable);
u8  CheckJNO(u32 jno);

uint16_t TitleTextLen = 3 ;
uint8_t  TitleText[30]={0x91,0x98,0xB9,0x70,0x1A,0xFF}; 

uint8_t  HighLightItem	= 0 ;	

uint8_t  ItemNum	= 6 ;

uint8_t  ItemIndex1 = 0x01;	
uint8_t  Item1_Icon = 0x01;
uint8_t  Item1_LineFeed = 0x00;	
uint16_t Item1_Reserved = 0x00;
uint16_t Item1_ItemLen = 3;	
uint8_t  Item1_Text[30] = {0x3A,0x67,0x66,0x8F,0x1A,0xFF};
	
uint8_t  ItemIndex2 = 0x02;	
uint8_t  Item2_Icon = 0x02;
uint8_t  Item2_LineFeed = 0x00;	
uint16_t Item2_Reserved = 0x00;
uint16_t Item2_ItemLen = 3;	
uint8_t  Item2_Text[30]  = {0x17,0x52,0x3E,0x5C,0x1A,0xFF};

uint8_t  ItemIndex3 = 0x03;	
uint8_t  Item3_Icon = 0x03;
uint8_t  Item3_LineFeed = 0x00;	
uint16_t Item3_Reserved = 0x00;
uint16_t Item3_ItemLen =   3;	
uint8_t  Item3_Text[30] = {0xCD,0x64,0x5C,0x4F,0x1A,0xFF};


uint8_t  ItemIndex4 = 0x04;	
uint8_t  Item4_Icon = 0x04;
uint8_t  Item4_LineFeed = 0x00;	
uint16_t Item4_Reserved = 0x00;
uint16_t Item4_ItemLen =   3;	
uint8_t  Item4_Text[30] = {0xCE,0x98,0x8B,0x53,0x1A,0xFF};

uint8_t  ItemIndex5 = 0x05;	
uint8_t  Item5_Icon = 0x05;
uint8_t  Item5_LineFeed = 0x00;	
uint16_t Item5_Reserved = 0x00;
uint16_t Item5_ItemLen =   3;	
uint8_t  Item5_Text[30] = {0xF6,0x65,0xF4,0x95,0x1A,0xFF};


uint8_t  ItemIndex6 = 0x06;	
uint8_t  Item6_Icon = 0x06;
uint8_t  Item6_LineFeed = 0x00;	
uint16_t Item6_Reserved = 0x00;
uint16_t Item6_ItemLen =   3;	
uint8_t  Item6_Text[30] = {0x3A,0x57,0x3A,0x5F,0x1A,0xFF};

uint16_t Button1Value =  0x0000 ;
uint16_t Button2Value =  0x0000 ;

char   DisTemp[30];

uint8_t  Dmr400MReceFrame[21] = {0xA5,0x5A,0x11,0xCA,0x10,0x44,0x00,0x01,0x5D,0x92,0x08,0x88,0x12,0x34,0x56,0x5D,0x9A,0xB3,0x42,0x00,0x23};

uint8_t Dmr400M_DataReceFlag = 0 ;	
uint8_t Dmr400M_DataReceData(uint8_t *recedata)  ;
/*--------------------------------------------------------------------------------------
  External Function Prototype ( declaration of functions which can be used externally )
  Example: U8 xxx_xxx( void );
---------------------------------------------------------------------------------------*/
void EnterGobUserMode(void);
void Sys_Soft_Reset(void);
void RadioSelfReset(void);
extern bool MasterRecordCfg(void);
void api_zzt_send_csbk_data(uint8_t *buf);
bool RadioSendCsbkFunction(uint8_t *csbkdataone,uint8_t *csbkdatatwo);
uint8_t CovertKeyValue(uint8_t KeyType);
	
bool MenuWindows_Start(uint32_t WindowID) ;	
bool MenuWindows_Opration(uint8_t SendExternCommandIndex,uint8_t SendCommandOprationIndex);

	
uint8_t Dmr400M_DataReceData(uint8_t *recedata)	;	
uint8_t Dmr400M_DataSendData(uint8_t command)	;
		
extern uint8_t T1S_Flag ;
extern uint8_t T1s_Flag3 ;

KeyActive_t KeyActiveStru;

ChannelNoti_t ChannelNotiStru;

uint8_t SpkOpenFlag = 0 ;
uint8_t SpkOpenCount = 10 ;

uint8_t  LinkStatus = 0 ;
	
uint32_t   JNO =  0xAAAAAAAA;
uint8_t    CommandH =  0x5D ;
uint8_t    CommandL =  0x92 ;
u16         COMMD ;
u16         PARA ;
uint16_t   FengYaZhi = 0x0888 ;
uint8_t    ParaH = 0x08 ;
uint8_t    ParaL = 0x88 ;
uint32_t   IDNUM = 0xFFFFFF ;
uint32_t   ReceTime = 0xFFFFFFFF ;
uint8_t    Rssi = 0xFF ;
uint32_t   EditIDNUM = 0x00 ;

u8 ModemData[16] ;
extern u8 ModemSendData[16]; 
bool ModemReceOK ;
uint32_t  SendIDNUM = 0xFFFFFF ;
uint32_t  SetIDNUM = 0xFFFFFF ;

//接收变量
u32 ReceJNO = 0 ;
u32 ReceIDNUM = 0 ;
u32 Rece5BIDNUM = 0 ;
u8  ReceCommandH = 0 ;
u8  ReceCommandL = 0 ;
u8  ReceKuJianCommandL = 0 ;
u8  ReceParaH = 0 ;
u8  ReceParaL  = 0 ;
u16 FengYaValue = 0xFFFF;
signed char  RSSILevel_400M = 0xFF ;
signed char  RSSILevel_450M = 0xFF ;

uint8_t  BackLinghtStatus = 1 ;
uint8_t  BackLinghtCount = 15 ;


uint8_t  Dmr400M_SendCmdChanl = 2 ;
uint8_t  Dmr400M_ReceCmdChanl = 1 ;

uint8_t  Dmr400M_SendCommandFlag = 0 ;
uint8_t  Dmr400M_SendCommandCount = 0 ;

uint8_t  Dmr400M_ReceCommandFlag = 0 ;
uint8_t  Dmr400M_LastReceCommand = 0 ;
uint8_t  Dmr400M_ReceCommandCount = 0 ;


#define  WaitQueRen_SetIpPara      7


uint8_t  SetIpIndex = 0 ;
uint8_t  SetIPParaValueTemp[17] = {0x00} ;
uint8_t  SetDateParaValue[4] ={0x00} ;


#define  WaitQueRen_SetJno         1
#define  WaitQueRen_SetIdnum       2 
#define  WaitQueRen_PaiFeng        3 
#define  WaitQueRen_XiaoHao        4 
#define  WaitQueRen_FengBao        5 
#define  WaitQueRen_DianBao        6
#define  WaitQueRen_Set5BIdnum     7

#define  WaitQueRen_SetAPNPara      0xA1    //设置APN
#define  WaitQueRen_SetMDNSPara     0xA2    //设置主用DNS
#define  WaitQueRen_SetSDNSPara     0xA3    //设置备用DNS
#define  WaitQueRen_SetRMSPara      0xA4    //设置备用RMS
#define  WaitQueRen_SetPORTPara     0xA5    //设置端口
#define  WaitQueRen_SetNetIPPara    0xA6    //设置网络IP
#define  WaitQueRen_SetIdnumPara    0xA7    //列尾ID设置

#define  WaitQueRen_SetDatePara     0xA8     //列尾日期设置
#define  WaitQueRen_SetTimePara     0xA9     //列尾时间设置

#define  WorkMode_ShuZi            1
#define  WorkMode_MoNi             2

#define  WorkFunc_QueRenYi         1
#define  WorkFunc_JianCeYi         2
#define  WorkFunc_KuJianYi         3


uint8_t  StartWaitQueRenFlag = 0 ;
uint8_t  StartWaitQueRenCount = 10 ;
uint8_t  CSBKFrameIndex = 0;

uint8_t  CommandIdleTimeCount = 0 ;
uint8_t  RadioJianTingMode = 1 ;
uint8_t  DebugMode = 1 ;

uint8_t   CurrentWorkMode = WorkMode_MoNi ;
uint8_t   CurrentWorkFunc = WorkFunc_QueRenYi ;

#define   AudioToMic   0x01 
#define   AudioToOB    0x00 

#define		RUN_LED_OFF				GPIO_ResetBits(GPIOB, GPIO_Pin_13); 			  //PA8 输出低电平，使其发光
#define		RUN_LED_ON				GPIO_SetBits(GPIOB,   GPIO_Pin_13); 			  //PA8 输出高电平，使其灭

bool    OBMicToRF_OutRouting_Switch(u8  para);

uint8_t ReadRadio_CurrentZone(void);
void ShowDispJNO(u32 tempjno);

uint8_t  PTTKeyKeepTime = 0 ;
u32        WindowID = 0x01;
extern void IWDG_Configuration(void);

bool GetRssiValueCommand(void);

//u32  Factory_Code ;
//u32  Factory_CodeArray[10] = {0x98761234,0x98763333,0x95763232,0x98763131,0x88763031,0x98761234,0x98763333,0x95763232,0x98763333,0x95763232} ; 

u16 RTC_Year ;
u8  RTC_Month ;
u8  RTC_Date ;
u8  RTC_Hour ;
u8  RTC_Minute ;
u8  RTC_Second ;

uint8_t   SendCommandFlag = 0x0 ;
#define		AudioOut_Sel_FSK()			GPIO_ResetBits(GPIOA , GPIO_Pin_8); 			//语音输出信号选择为FSK
#define		AudioOut_Sel_SPK()			GPIO_SetBits(GPIOA , GPIO_Pin_8); 			  //PA8 输出高电平，使其灭

u8     WorkMode_LwOrCall = 1 ;
u8     KeyFunVlaue = 0 ;
u8     AudioMuteFLag = 0 ;
u8     ReceOneFrameData = 0 ;
u8     StartCallDriverYaLinFlag = 0 ;
u8     StartCallDriverYaLinCount = 0 ;
u8     WorkXinDaoNum = 1  ;
void   KeyValueManagement(u8 keyvalue) ;
bool KeyActiveFuncManagement(void) ;

extern uint16_t gHrnpRxDataPN ;

u8  SendExternCommandIndex = 0xFF ;
u8  SendCommandOprationIndex = 0 ;

u8  SendCommandArray[5] = {0xD3,0xD5,0xD7,0xD9,0xDA} ;

static u32 LastReceTime= 0 ;
u8   KeyActtiveCount = 0 ;
u16  SoftVersion = 0x06 ;
u8   SendDAD9Flag = 0 ;
u8   MenuWindows_DisOk = 0 ;
void MenuWindows_Function(void);
u8* MenuWindows_SetIPPara(u8 Funsel);
u32 MenuWindows_SetIP(u8 Funsel);
void MenuWindows_Refresh(void);
void MenuWindows_Clear(void);
bool MenuWindows_MoudleStatus(u16 FengYaValue,u32 ReceTime) ;
bool MenuWindows_GSMRStatus(u16 FengYaValue,u32 ReceTime) ;
void MenuWindows_ParaAckMenu(u8 command,u16 FengYaValue,u32 ReceTime);
void MenuWindows_ParaSendMenu(u8 command,u16 FengYaValue,u32 ReceTime);
void MenuWindows_SetIDPara(void);

uint8_t Dmr400M_DataSendDataFun(uint8_t command);

uint8_t Dmr400M_DataReceDataFun(uint8_t *recedata);

u32 MenuWindows_SetTime(void);
u32 MenuWindows_SetAPN(void);
u32 MenuWindows_SetPort(void);
u32 MenuWindows_PassWord(void);


/***************************************************************************
Function name   :EnterGobUserMode
Description     :Gob user APP mode entry 
Input           :none 
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
void EnterGobUserMode(void)
{	
	  uint16_t BKP_DR2Value ;
	  uint16_t Int16Value1 ;
	  uint16_t Int16Value2 ;
		u32      KaiJiTime = 1 ;
	  u8       i = 1 ;
	  u16     NoKeyActivtyCount = 0 ;
	  bool result = FALSE;
		
		printf("\r\n OB Broad Init !!!\r\n") ;
		
/*  recording function configuration  */
    RadioOBInit();
		
    //初始化选件板和终端的	
		AudioSpkConInit();	
	
		BKP_DR2Value = BKP_ReadBackupRegister(BKP_DR2);	

		if(BKP_DR2Value==0xA6A6)                  //异常复位
		{
          printf("\r\n BKP_DR2Value Is %04X, Reset occurred By self.... \r\n",BKP_DR2Value);
		}
		else if(BKP_DR2Value==0xA5A5)            //系统复位
		{
					printf("\r\n BKP_DR2Value Is %04X, Reset occurred.... \r\n",BKP_DR2Value);
		}
		else                                    //系统上电
		{
					printf("\r\nBKP_DR2Value Is %04X,Power On occurred....\r\n",BKP_DR2Value);
		}

	  HrnpSlaveRcvAndOperateCmdFromMaster() ;    //接收处理函数
		
		wtn_set_volume(16) ;                      				  //设置语音芯片音量幅度
		
		printf("\r\n Init OB Broad Ok !\r\n") ;
		
	  HrnpSlaveRcvAndOperateCmdFromMaster() ;    //接收处理函数
	
		CurrentWorkMode = WorkMode_ShuZi ;
		
		WorkXinDaoNum = 11 ;
		
		Dmr400M_ReceCmdChanl = 11 ;	
		
		Dmr400M_SendCmdChanl = 12 ;
			
		for(i=0;i<3;i++)
		{
					 result = RadioChange_Zone(0,1,WorkXinDaoNum);    //数字发送信道回归到接收信道
					 if(result==TRUE)
					 {
							  break ;
					 }
		}
				
    HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
		
		AudioSpkOpen() ;

		if(BKP_DR2Value!=0xA6A6)                               //系统上电
		{	
						SpkOpenCount = 5 ;
						wtn_play_voice(0xA4) ;                         // “列尾确认仪语音”语音
						BKP_WriteBackupRegister(BKP_DR2,0xA6A6) ;	
		}
		else
		{
						printf("\r\n BKP_DR2Value Is %04X,Self Reset occurred.... \r\n",BKP_DR2Value) ;        //  自己控制的复位
						Int16Value1 = BKP_ReadBackupRegister(BKP_DR5) ;
						LinkStatus = Int16Value1 ;
						if(LinkStatus==1)
						{
											Int16Value1 = BKP_ReadBackupRegister(BKP_DR3);
											Int16Value2 = BKP_ReadBackupRegister(BKP_DR4);
									
											IDNUM = Int16Value1<<16|Int16Value2 ;
											IDNUM = IDNUM&0xFFFFFF ;
									
											Int16Value1 = BKP_ReadBackupRegister(BKP_DR6);
											Int16Value2 = BKP_ReadBackupRegister(BKP_DR7);
											JNO = Int16Value1<<16|Int16Value2 ;
									
											printf("\r\n FlashAdd Read JNO Value %8X .\r\n",JNO);
									
											if(CheckJNO(JNO)==0)   
											{
														JNO = 0xAAAAAAAA ;
														IDNUM = 0xFFFFFF ;
														LinkStatus = 0 ;
											}
						}
						else
						{
											LinkStatus = 0 ;
											JNO = 0xAAAAAAAA;	
											IDNUM = 0xFFFFFF;	
											
									
						}
		}
		
		MenuWindows_DisOk = 0 ;
	
	  MenuWindows_Start(WindowID) ;
		
    HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
				
	  SendCommandFlag = 0x00 ;
		
		KeyActiveStru.KeyOperate = 0xFF ;
		
		DebugMode = 0x01 ;
		
		BackLinghtStatus = 1 ;
		
		BackLinghtCount = 29 ;	
		
	 	WindowBackLightCon(1) ;	
		
	  while(1)
    {
			    HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	

					if(KeyActiveStru.KeyOperate!=0xFF)	
					{				       
									 CurrentWorkMode = WorkMode_ShuZi ;
									 WorkXinDaoNum = 11 ;
									 Dmr400M_ReceCmdChanl = 11 ;	
									 Dmr400M_SendCmdChanl = 12 ;
						       KeyActtiveCount = 10 ;  //1次按键10秒内不接受400M数据
						
						       SpkOpenCount = 30 ;
						
								   KeyActiveFuncManagement() ;	
						
						       NoKeyActivtyCount = 0;
						
									 KeyActiveStru.KeyOperate = 0xFF ; 						
					}
								
					HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
					
					if(SendCommandFlag!=0)
					{	
						       CommandIdleTimeCount = 0 ;
						       KeyActtiveCount = 0 ;
									 ModemReceOK = FALSE ;
									 Dmr400M_ReceCommandFlag = 0 ;
									 Dmr400M_DataSendData(SendCommandFlag) ;
									 SendCommandFlag = 0 ;
						       StartWaitQueRenFlag = 0 ;
						       Dmr400M_DataReceFlag = 0 ;
						       LastReceTime = 0 ;
					}	
					if(T1s_Flag3==1)
					{
						
									 CommandIdleTimeCount++ ;
						
							     printf("\r -%d \n",CommandIdleTimeCount);
						
									 if(CommandIdleTimeCount>30||gConnectStateFlag==OFF)
									 {
										     if(StartWaitQueRenFlag==0)
												 {
														    RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;    //设置当前信道
												 }	 
												 
												 CommandIdleTimeCount = 0  ;
									 }
									 
									 if(KeyActtiveCount>0)
									 {
										     KeyActtiveCount-- ;
									 }
									 
									 
									 NoKeyActivtyCount++;
									 
									 if(NoKeyActivtyCount>600)
									 {
									      LinkStatus = 0 ;
										    JNO = 0xAAAAAAAA ;
										    IDNUM = 0xFFFFFF ;
										    NoKeyActivtyCount = 0 ;
									 }
									 
										if(LinkStatus==1)
										{
														RadioJianTingMode = 0 ;  
										}
										else if(IDNUM<=0xA00000||JNO<=0xA0000000)
										{
														RadioJianTingMode = 0 ;      
										}
										else if(CheckJNO(JNO)==1)
										{
										        RadioJianTingMode = 0 ;      
										}
										else
										{ 
											     if(RadioJianTingMode==0)
													 {
														    RadioJianTingMode = 1 ; 
														 
													 }														 
										}
												
									  T1s_Flag3 = 0 ;			 
					}
					
				 HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
				 if(StartWaitQueRenFlag==0&&WorkMode_LwOrCall==1&&KeyActtiveCount==0)
				 {
										if(Dmr400M_DataReceFlag==1)    
										{		     
												   Dmr400M_DataReceData(Dmr400MReceFrame+2);   //注意
													 CommandIdleTimeCount = 0x00 ;
													 Dmr400MReceFrame[3]  = 0xFF ;
													 Dmr400MReceFrame[9]  = 0xFF ;
													 Dmr400M_DataReceFlag = 0x00 ;
										}

										HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					}
					
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
					  if(T1S_Flag==1)
						{  
							           
													RUN_LED__TURN ;
													IWDG_ReloadCounter();    //喂狗
													HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
													if(StartWaitQueRenFlag!=0)    //机车台等待确认不操作恢复
													{
														       Dmr400M_DataReceFlag = 0 ;
																	 if(StartWaitQueRenCount>0)
																	 {
																					 StartWaitQueRenCount-- ;
																					 if(StartWaitQueRenCount==0)
																					 {
																									HighLightItem	= 0 ; 
																									Button1Value =  0x0000 ;
																									Button2Value =  0x0000 ;
																									MenuWindows_Start(WindowID) ;
																									StartWaitQueRenFlag = 0 ;
																						 
																					 }
																	 }
													}

													HrnpSlaveRcvAndOperateCmdFromMaster() ;                    //接收处理函数
													
													if(Dmr400M_ReceCommandFlag==1)
													{			
																		Dmr400M_ReceCommandCount++; 
																		Dmr400M_SendCommandFlag = 0 ;
																		Dmr400M_SendCommandCount = 0 ;
																		if(LinkStatus==0)
																		{
																								if(Dmr400M_LastReceCommand==0x5B)   
																								{
																														if(Dmr400M_ReceCommandCount>=5)    //显示接收画面清空
																														{
																																		Dmr400M_ReceCommandFlag = 0 ;
																																		Dmr400M_ReceCommandCount = 0 ;																				
																																		Dmr400M_LastReceCommand = 0 ;
																																		Dmr400M_SendCommandFlag = 0 ;
																																		Dmr400M_SendCommandCount =  0 ;
																																		MenuWindows_Start(WindowID) ;
																														}
																								}
																								else
																								{
																														if(Dmr400M_ReceCommandCount>=5)
																														{
																																		Dmr400M_ReceCommandFlag = 0 ;
																																		Dmr400M_ReceCommandCount = 0 ;
																																		Dmr400M_LastReceCommand = 0 ;
																																		Dmr400M_SendCommandFlag = 0 ;
																																		Dmr400M_SendCommandCount =  0 ;
																																		MenuWindows_Start(WindowID) ;
																														}
																								}
																		}
																		else 
																		{
																								if(Dmr400M_ReceCommandCount>=5)   //显示接收画面清空
																								{																					 
																												Dmr400M_ReceCommandFlag = 0 ;
																												Dmr400M_ReceCommandCount = 0 ;																				
																												Dmr400M_LastReceCommand = 0 ;
																												Dmr400M_SendCommandFlag = 0 ;
																												Dmr400M_SendCommandCount =  0 ;
																												MenuWindows_Start(WindowID) ;
																								 }
																		}	
													 }
													 else if(Dmr400M_SendCommandFlag==1)
													 {
																			Dmr400M_SendCommandCount++ ; 
																			if(Dmr400M_SendCommandCount>=3)
																			{
																						Dmr400M_SendCommandFlag =  0 ;
																						Dmr400M_SendCommandCount =  0 ;
																						if(LinkStatus==0)
																						{
																								 SendIDNUM = IDNUM ;
																						}
																						MenuWindows_Start(WindowID) ;
																			}
													 }
													 
													 T1S_Flag = 0 ;									 
							}
					
		}
		
}

void   KeyValueManagement(u8 keyvalue) 
{
	
//			 u8 i = 0 ;
//	     bool  Result = FALSE ;
//	
//			 if(keyvalue==KEY_FUN_VALUE)
//			 {
//				           WorkMode_LwOrCall = 1;
//				           KeyFunVlaue = 0 ;      
//									 MenuWindows_Start(WindowID) ;                    // OB板取消SPK独占模式	
//									 RadioSpkCon(1);			                             // OB板取得SPK独占模式
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 AudioSpkMute(0);
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数								 
//									 OBMicToRF_OutRouting_Switch(AudioToOB) ;           //模拟参数配置
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 RadioChange_Zone(0,1,1);                          //模拟参数配置	
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 SpkOpenFlag = 0 ;  
//									 AudioSpkOpen() ;
//				           Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 SpkOpenCount = 2 ;
//				           delay_ms(100) ;
//									 wtn_play_voice_one(69);                          //列尾
//				           Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 PTTKeyKeepTime = 0 ;
//									
//			 }
//			 else if(keyvalue==KEY_SK1_VALUE)
//			 {
//				           SpkOpenFlag = 0;
//					         AudioSpkOpen();
//				 					 if(WorkMode_LwOrCall!=2)
//									 {
//										   MenuWindowsClose(WindowID) ;                  // 关闭显示窗口
//									 }
//									 RadioChange_Zone(0,1,1);                         //切换信道
//									 
//									 wtn_play_voice_one(57);                           //呼叫
//									 
//									 for(i=0;i<30;i++)
//									 {
//												Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//												delay_ms(20);
//										    IWDG_ReloadCounter();    //喂狗
//									 }
//									 RadioSpkCon(0);
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;   //接收处理函数	 
//								   OBMicToRF_OutRouting_Switch(AudioToMic) ;          //模拟参数配置
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;   //接收处理函数	
//									 AudioSpkUnMute(0);
//								   IWDG_ReloadCounter();    //喂狗
//								   Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;   //接收处理函数	
//									 PTTKeyKeepTime = 60 ;
//									 KeyFunVlaue = 0 ;
//									  WorkMode_LwOrCall = 2 ; 
//									 delay_ms(50);
//									 IWDG_ReloadCounter();    //喂狗
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 StartCallDriverYaLinFlag = 1 ;
//                   StartCallDriverYaLinCount = 0 ;
//									 
//				}
//				else if(keyvalue==KEY_SK2_VALUE)
//				{
//					         SpkOpenFlag = 0;
//					         AudioSpkOpen();
//					         if(WorkMode_LwOrCall!=2)
//									 {
//										     MenuWindowsClose(WindowID) ;               // 关闭显示窗口	
//									 }
//									 
//									 wtn_play_voice_one(57) ;                     //呼叫
//									 
//									 RadioChange_Zone(0,1,2) ;                   //切换信道
//									 
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 for(i=0;i<30;i++)
//									 {
//												Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//												delay_ms(20) ;
//												IWDG_ReloadCounter();    //喂狗
//									 }
//									 
//									 RadioSpkCon(0) ;
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
//									 OBMicToRF_OutRouting_Switch(AudioToMic) ;         //模拟参数配置
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
//									 AudioSpkUnMute(0) ;
//									 IWDG_ReloadCounter();                              //喂狗
//									 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数									 
//									 PTTKeyKeepTime = 60 ;
//								   KeyFunVlaue = 0 ;
//									 WorkMode_LwOrCall = 2 ;
////								 RadioPttCon(1) ; 
//  								 IWDG_ReloadCounter();    //喂狗
//  								 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
//									 StartCallDriverYaLinFlag = 1 ;
//									 StartCallDriverYaLinCount = 0 ;	 
//					}
}
u8  CheckJNO(u32 jno)
{			
	      u8 i = 0 ;
	
	      u8 temp = 0 ;
	
				for(i=0;i<8;i++)
				{
							temp = (jno>>i*4)&0x0F ;
							if(temp>=0x0A)
							{
										return 0 ;
							}
				}
			  return  1 ;
}

uint8_t Dmr400M_DataReceData(uint8_t *recedata)
{
				 u8   i = 0;
				 u8   TempReceData[50] ;
				 u32  TempJNO= 0  ;
				 u8   ReceCount = 0 ;  
				 u16  year;
				 u8   mon;
				 u8   mday;
				 u8   hour;
				 u8   min;
				 u8   sec ;
				 u8   CommandSearchFailFlag = 0 ;
				 u32  TempInt = 0 ;
	       static u8 LastComL ;
	       static u16 LastFengYaZhi ;

				 static u8  AvoidReceSameDataCount = 0 ;
	       u8     JNOClearFlag = 0;

					ReceCount = recedata[0] ;             //数据长度超长 
					 
					TempJNO = JNO ;
					 
					for(i=0;i<(ReceCount+3);i++)
					{			  
								TempReceData[i] = recedata[i]  ;					              		 
					}
				 
				  printf("\r\n Dmr_Moudle Data!\r\n");
				 	printf("\r\n A5 5A");				 
					for(i=0;i<=(ReceCount+1);i++)
					{	
								printf(" %02X",TempReceData[i]);			
					}
					printf("\r\n");

					if(TempReceData[1]==0xCA)    //数字指令
					{			
											ReceJNO = TempReceData[2]<<24|TempReceData[3]<<16|TempReceData[4]<<8|TempReceData[5] ;
											ReceCommandH = TempReceData[6] ;
											ReceCommandL = TempReceData[7] ;
											ReceParaH =  TempReceData[8] ;
											ReceParaL =  TempReceData[9] ;
						          FengYaZhi = ReceParaH<<8|ReceParaL ;	
						          FengYaValue = TempReceData[8]<<8|TempReceData[9] ;
											ReceIDNUM = TempReceData[10]<<16|TempReceData[11]<<8|TempReceData[12] ;				 
											ReceTime  =  TempReceData[13]<<24|TempReceData[14]<<16|TempReceData[15]<<8|TempReceData[16] ;
											RSSILevel_400M = TempReceData[17];
						
											year =  (ReceTime>>26)&0x3F;
											mon =   (ReceTime>>22)&0x0F; 
											mday =  (ReceTime>>17)&0x1F;
											hour = (ReceTime>>12)&0x1F;
											min=  (ReceTime>>6)&0x3F;
											sec=  ReceTime&0x3F;
						
											if(LastReceTime==ReceTime&&LastComL==ReceCommandL&&LastFengYaZhi==FengYaZhi)
											{			
												        AvoidReceSameDataCount++ ;
												        printf("\r\nDmr400M Receive Same DataFrame---%d Time!!!\r\n",AvoidReceSameDataCount) ; 
												        if(AvoidReceSameDataCount>7)
																{
																	   AvoidReceSameDataCount = 0 ; 
																     LastReceTime = 0xFFFFFFFF ;
																}
																 return 0;
											}
											else
											{
																AvoidReceSameDataCount = 0 ;
																LastComL = ReceCommandL ;
																LastReceTime = ReceTime ;	
												        LastFengYaZhi = FengYaZhi ;
											}
											
											printf("\r ReceJNO: %08X \n",ReceJNO);
											printf("\r ReceIDNUM: %06X \n",ReceIDNUM);
											printf("\r COMMAND: %02X%02X \n",ReceCommandH,ReceCommandL);
						          printf("\r PARA: %02X%02X \n",ReceParaH,ReceParaL);
	

											if(year>23&&year<36)
											{	
												     printf("\r TIME: %02d-%02u-%02u  %02u:%02u:%02u \n",year,mon,mday,hour,min,sec);	
											}
										
											printf("\r RSSI: %02d dB\n", RSSILevel_400M);

										  if(LinkStatus==0)	         //未连接状态
											{
																 if(JNO==ReceJNO)
																 {
																					if(ReceCommandL==0x92||ReceCommandL==0xB6)
																					{
																								 if(Dmr400M_SendCommandFlag==1&&IDNUM==ReceIDNUM)
																								 {
																												 LinkStatus = 1 ;
																												 TempInt = (IDNUM>>16)&0xFFFF;
																												 BKP_WriteBackupRegister(BKP_DR3,TempInt);
																												 TempInt = (IDNUM)&0xFFFF;
																												 BKP_WriteBackupRegister(BKP_DR4,TempInt);
																												 TempInt = LinkStatus ;
																												 BKP_WriteBackupRegister(BKP_DR5,TempInt);
																								 }		 
																					}		
																 }
															   else if(ReceIDNUM==IDNUM&&IDNUM!=0xFFFFFF)
																 {	 
																 					Button1Value =  0x0000 ;
																					Button2Value =  0x0000 ;
   																	      HighLightItem	=  0;	
																 }
																 else if(ReceCommandL==0x5B&&IDNUM>=0xA00000)
																 {
																	        StartWaitQueRenFlag = WaitQueRen_Set5BIdnum ;
																	        StartWaitQueRenCount = 5 ;
																	        Rece5BIDNUM = ReceIDNUM ;
																			 		Button1Value =  0x0001 ;
																					Button2Value =  0x0002 ;		
																					HighLightItem	= 2 ;			
																 }
																 else if(RadioJianTingMode==1)     //监听模式
																 {										   
																				 if(CheckJNO(JNO)==1)
																				 {
																							 return 0 ;  	 
																				 }	
																 } 
																 else
																 {
																				 return 0 ;
																 }
											 }
											 else if(LinkStatus==1)         //已连接状态
											 {		
																	if(JNO==ReceJNO&&IDNUM==ReceIDNUM)
																	{
																					if(ReceCommandL==0xB6||ReceCommandL==0x92)
																					{
																							 LinkStatus = 1 ;
																					}																				
																					else if(ReceCommandL==0x84||ReceCommandL==0x85)
																					{
																							 IDNUM = 0xFFFFFF ;
																							 LinkStatus = 0 ;
																						   JNOClearFlag  = 1 ;
																						   RadioJianTingMode = 1 ;
																							 TempInt = (IDNUM>>16)&0xFFFF;
																							 BKP_WriteBackupRegister(BKP_DR3,TempInt);
																							 TempInt = (IDNUM)&0xFFFF;
																							 BKP_WriteBackupRegister(BKP_DR4,TempInt);	
																							 TempInt = LinkStatus ;
																							 BKP_WriteBackupRegister(BKP_DR5,TempInt);
																					}													
																	}
																	else if(ReceCommandL==0x5B&&IDNUM==ReceIDNUM) 
																	{
																							
																	}
																	else
																	{
																							 return 0 ;		
																	}
											  }
						}
						else
						{
									return 0 ;			
						}
						
						 for(i=0;i<40;i++)
						 {
									 HrnpSlaveRcvAndOperateCmdFromMaster() ;     //等待其他数据帧接收完成'
									 delay_ms(10);
									 IWDG_ReloadCounter();    //喂狗
					   }
						
					  CSBKFrameIndex = 0 ;
						Dmr400M_LastReceCommand = ReceCommandL ;
						Dmr400M_ReceCommandFlag = 1 ;
            Dmr400M_ReceCommandCount = 0 ;
						Dmr400M_SendCommandFlag = 0 ;
            Dmr400M_SendCommandCount = 0 ;
						
						if(BackLinghtStatus==0)
						{
									BackLinghtStatus = 1 ;   
									WindowBackLightCon(1); 						
						}
						
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
						if(ReceCommandL==0x5B&&LinkStatus==0)
						{
					       BackLinghtCount = 30 ;  // 输号请求等待的时间久一点
						}
						else
						{
						     BackLinghtCount = 15 ;	 // 普通命令的时间久一点
						}
						
						//测试接收模式						
						if(ReceCommandL==0x5B)
						{
							    TempJNO = JNO  ;
						      ShowDispJNO(JNO);
						}
						else
						{
									TempJNO = JNO  ;
									JNO = ReceJNO  ;
									ShowDispJNO(ReceJNO);
						}
						
						Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3 ;
						
					  sprintf(DisTemp,"%06X",ReceIDNUM);
					  Item2_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;
						
						sprintf(DisTemp,"%s","          ");	
						SpkOpenCount = 7 ;	
					  switch(ReceCommandL)
						{		
									 case 0x01:
											   if(ReceParaH==0x01&&ReceParaL==0x00)
												 {
                              sprintf(DisTemp,"%s","关闭成功");
                              ReceCommandL = 0 ;													 
												 }
												 else if(ReceParaH==0x01&&ReceParaL==0x01)
                         {
												      sprintf(DisTemp,"%s","开启成功");	
													    ReceCommandL = 1 ;			
												 }
									  break ;			 
									  case 0x02:
								         sprintf(DisTemp,"%s","状态应答");
									  break ;
										case 0x24:
												 sprintf(DisTemp,"%s","待排风");		
										break ;
										case 0xB5:
												 sprintf(DisTemp,"%s","确认");
										break ;								
										case 0x91:
												 sprintf(DisTemp,"%s","风压查询");
										break ;
										case 0x93:
												 sprintf(DisTemp,"%s","自动风压");
										break ;
										case 0x83:
												sprintf(DisTemp,"%s","销号");
										break ;
										case 0x31:
												sprintf(DisTemp,"%s","风警确认");
										break ;
										case 0xB1:
												sprintf(DisTemp,"%s","电警确认");
										break ;
										case 0x5B:	
												 sprintf(DisTemp,"%s","输号请求");		
										break ;
										case 0x25:
												 sprintf(DisTemp,"%s","排风开始");		
										break ;
										case 0xB6:
												 sprintf(DisTemp,"%s","确认完毕");
										     SpkOpenCount = 13 ;	
										break ;
										case 0xB7:
												 sprintf(DisTemp,"%s","已连接其他机车");
										break ;								
										case 0x92:
												 sprintf(DisTemp,"%s","报风压  ");
										break ;
										case 0x94:
												 sprintf(DisTemp,"%s","自动风压");
										break ;
										case 0x30:
												 sprintf(DisTemp,"%s","风压报警");
										break ;  
										case 0xB0:
												 sprintf(DisTemp,"%s","电量不足");
										break ;
										case 0x84:
												sprintf(DisTemp,"%s","销号成功");
										    SpkOpenCount = 9 ;	
										break ;
										case 0x85:
												sprintf(DisTemp,"%s","已销号  ");
										break ;
										
										case 0xD4:
												sprintf(DisTemp,"%s","号码应答");
										break ;			
										case 0xD6:
												sprintf(DisTemp,"%s","模块状态");
										break ;
										case 0xD8:
												sprintf(DisTemp,"%s","网状状态");
										break ;
										case 0xDB:
												sprintf(DisTemp,"%s","参数应答");
										break ;
										
										
										default:	
												sprintf(DisTemp,"%02X%02X",ReceCommandH,ReceCommandL); 
												CommandSearchFailFlag = 1 ;
										break ;
						}
						
						if(CommandSearchFailFlag==1)
						{
						       Item3_ItemLen = AscllCovertUniCode(DisTemp,Item3_Text+6) + 3 ;	
                 					
						}
						else
						{		
		          	   Item3_ItemLen = HanZiCovertUniCode(DisTemp,Item3_Text+6) + 3 ;	
						}
						
						
						if(ReceCommandL<=0x02)
						{
										if(ReceParaL==0x00)
										{
												sprintf(DisTemp,"%s","卡故障     "); 
											  Item4_ItemLen = HanZiCovertUniCode(DisTemp,Item4_Text+6) + 3 ;
										}
										else if(ReceParaL==0x01)
										{
												sprintf(DisTemp,"%s","列尾地址  ");
                        Item4_ItemLen = HanZiCovertUniCode(DisTemp,Item4_Text+6) + 3 ;											
										}	
										else if(ReceParaL==0x11)
										{
												sprintf(DisTemp,"%s","机列地址  "); 
											  Item4_ItemLen = HanZiCovertUniCode(DisTemp,Item4_Text+6) + 3 ;			
										}
										else if(ReceParaL==0x22)
										{
												sprintf(DisTemp,"%s","注成端口正"); 
											   Item4_ItemLen = HanZiCovertUniCode(DisTemp,Item4_Text+6) + 3 ;			
										}	
										else if(ReceParaL==0x02)
										{
												sprintf(DisTemp,"%s","注成端口错"); 
											 Item4_ItemLen = HanZiCovertUniCode(DisTemp,Item4_Text+6) + 3 ;			
										}	
										else if(ReceParaL==0x03)
										{
												sprintf(DisTemp,"%s","等待注册  "); 
											  Item4_ItemLen = HanZiCovertUniCode(DisTemp,Item4_Text+6) + 3 ;			
										}	
										else if(ReceParaL==0x04)
										{
												sprintf(DisTemp,"%s","搜索网络  "); 
											  Item4_ItemLen = HanZiCovertUniCode(DisTemp,Item4_Text+6) + 3 ;			
										}	
										else 
										{
												 sprintf(DisTemp,"%04X  kPa ",FengYaZhi);	
							           Item4_ItemLen = AscllCovertUniCode(DisTemp,Item4_Text+6) + 3 ; 
										}		
										
						}
						 else
						 {
							 
												sprintf(DisTemp,"%03X  kPa ",FengYaZhi);
												Item4_ItemLen = AscllCovertUniCode(DisTemp,Item4_Text+6) + 3;
							
							          sprintf(DisTemp,"%02u/%02u %02u:%02u",mon,mday,hour,min);
												Item5_ItemLen = AscllCovertUniCode(DisTemp,Item5_Text+6) + 3;
							 		
												if(RSSILevel_400M==26)
												{
																	sprintf(DisTemp,"--- dBm");
																	Item6_ItemLen = AscllCovertUniCode(DisTemp,Item6_Text+6) + 3;						
												}
												else
												{
																	sprintf(DisTemp,"%d dBm",RSSILevel_400M);
																	Item6_ItemLen = AscllCovertUniCode(DisTemp,Item6_Text+6) + 3;					
												}
												
						}

						JNO = ReceJNO ;
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
						if(ReceCommandL==0x5B)
						{

						}
						else	
						{
								HighLightItem = 0 ;
								Button1Value =  0x0000 ;
								Button2Value =  0x0000 ;
						}
					
						MenuWindowsDis(WindowID);

						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
						if(CommandSearchFailFlag==1)
						{
						      CommandSearchFailFlag = 0 ;		
						}
						else
						{
									AudioSpkOpen();		
							
									wtn_play_voice(ReceCommandL);					
						}	
						
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数

            if(JNOClearFlag==1)
						{
						     JNO = 0xAAAAAAAA ;
						}
						else
						{
					    	 JNO = TempJNO  ;
						}

						MenuWindowsDis(WindowID);
}
uint8_t Dmr400M_DataReceDataFun(uint8_t *recedata)
{
				 u8   i = 0;
				 u8   TempReceData[50] ;
         u32  TempJNO= 0  ;
				 u8   ReceCount = 0 ;  
				 u16  year;
				 u8   mon;
				 u8   mday;
				 u8   hour;
				 u8   min;
				 u8   sec ;
				 u8   CommandSearchFailFlag = 0 ;

				 ReceCount = recedata[0] ;           //数据长度超长 
				 
				 TempJNO = JNO ;
				 
				 for(i=0;i<(ReceCount+3);i++)
				 {			  
								TempReceData[i] = recedata[i]  ;					              		 
				 }
				 
				  printf("\r\n Dmr_Moudle Data!\r\n");
				 	printf("\r\n A5 5A");				 
					for(i=0;i<=(ReceCount+1);i++)
					{	
								printf(" %02X",TempReceData[i]);			
					}
					printf("\r\n");

					if(TempReceData[1]==0xCA)    //数字指令
					{			
						
											ReceJNO = TempReceData[2]<<24|TempReceData[3]<<16|TempReceData[4]<<8|TempReceData[5] ;
											ReceCommandH = TempReceData[6] ;
											ReceCommandL = TempReceData[7] ;
											ReceParaH =  TempReceData[8] ;
											ReceParaL =  TempReceData[9] ;
						          FengYaZhi = ReceParaH<<8|ReceParaL ;	
						          FengYaValue = TempReceData[8]<<8|TempReceData[9] ;
											ReceIDNUM = TempReceData[10]<<16|TempReceData[11]<<8|TempReceData[12] ;				 
											ReceTime  =  TempReceData[13]<<24|TempReceData[14]<<16|TempReceData[15]<<8|TempReceData[16] ;
											RSSILevel_400M = TempReceData[17];
						

											year =  (ReceTime>>26)&0x3F;
											mon =   (ReceTime>>22)&0x0F; 
											mday =  (ReceTime>>17)&0x1F;
											hour = (ReceTime>>12)&0x1F;
											min=  (ReceTime>>6)&0x3F;
											sec=  ReceTime&0x3F;
											
											if(year>23&&year<36)
											{	
												
												     printf("\r TIME: %02d-%02u-%02u  %02u:%02u:%02u \n",year,mon,mday,hour,min,sec);	
												
											}

											printf("\r ReceJNO: %08X \n",ReceJNO);
											printf("\r ReceIDNUM: %06X \n",ReceIDNUM);
											printf("\r COMMAND: %02X%02X \n",ReceCommandH,ReceCommandL);
						          printf("\r PARA: %02X%02X \n",ReceParaH,ReceParaL);
						          printf("\r ReceTime: %08X%\n",ReceTime);
											printf("\r TIME: %08x \n",ReceTime);	
											printf("\r RSSI: %02d dBm\n",RSSILevel_400M);
											
										  if(ReceCommandL<0xD0)
											{
													  return 0;
											}

						}
						else
						{
									return 0 ;			
						}
						
						printf("\r ReceIDNUM: %06X  EditIDNUM: %06X \n",ReceIDNUM,EditIDNUM);
						
						if(ReceCommandL==0xD4)
						{
							   if(ReceIDNUM == EditIDNUM)
								 {
								 
								 }
								 else
								 {
											  printf("\r ReceIDNUM: %06X \n",ReceIDNUM);
											  IDNUM = ReceIDNUM ;
								 }
						}
						else if(ReceIDNUM!=IDNUM)
						{
						       return 0; 
						}
							

					  for(i=0;i<5;i++)
					  {
								 HrnpSlaveRcvAndOperateCmdFromMaster() ;     //等待其他数据帧接收完成'
								 delay_ms(10);
								 IWDG_ReloadCounter();    //喂狗
					  }
						
					  CSBKFrameIndex = 0 ;
						Dmr400M_LastReceCommand = ReceCommandL ;
						Dmr400M_ReceCommandFlag = 1 ;
            Dmr400M_ReceCommandCount = 0 ;
						Dmr400M_SendCommandFlag = 0 ;
            Dmr400M_SendCommandCount = 0 ;
						
						if(BackLinghtStatus==0)
						{
									BackLinghtStatus = 1 ;   
									WindowBackLightCon(BackLinghtStatus); 						
						}
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						if(ReceCommandL==0x5B&&LinkStatus==0)
						{
					       BackLinghtCount = 30 ;  // 输号请求等待的时间久一点
						}
						else
						{
						     BackLinghtCount = 15 ;	 // 普通命令的时间久一点
						}
						
						SpkOpenCount = 7 ;	

            if(ReceCommandL==0xD4)
						{
							      Item2_ItemLen = 3 ;
							      HighLightItem = 2 ;
							      sprintf(DisTemp,"设置成功");
							           
							      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
						}
						else if(ReceCommandL==0xD6)
						{
									 Item2_ItemLen = 3;
							     HighLightItem = 2 ;
							     sprintf(DisTemp,"查询成功");
							     Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
                   MenuWindows_MoudleStatus(FengYaValue,ReceTime);

						}
						else if(ReceCommandL==0xD8)
						{
										sprintf(DisTemp,"查询成功");
							      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
							      MenuWindows_GSMRStatus(FengYaZhi,ReceTime);
						}
						else if(ReceCommandL==0xDB)
						{
										 if(SendDAD9Flag==0xDA) 
										 {
												 sprintf(DisTemp,"设置应答");
										 }
										 else if(SendDAD9Flag==0xD9) 
										 {
										     sprintf(DisTemp,"查询应答");	 
										 }
							      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
										 
							      MenuWindows_ParaAckMenu(ReceCommandL,FengYaValue,ReceTime);
						}

						HighLightItem = 2 ;
						Button1Value =  0x0000 ;
						Button2Value =  0x0001 ;
					
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						 
						MenuWindowsDis(WindowID);
						 
						
						if(CommandSearchFailFlag==1)
						{
						      CommandSearchFailFlag = 0 ;		
						}
						else
						{
										 if(SendDAD9Flag==0xDA) 
										 {
												 
										 }
										 else if(SendDAD9Flag==0xD9) 
										 {
										     ReceCommandL++;	 
										 }
								 	AudioSpkOpen();	
                  if(ReceCommandL<4)	
									{
										  wtn_play_voice_one(91);
										  wtn_play_voice_one(37);
									}	
									else
									{
												wtn_play_voice(ReceCommandL);					
									}	
								  
						}	
						
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数

            JNO = TempJNO ;
						
						for(i=0;i<80;i++)
						{
									 HrnpSlaveRcvAndOperateCmdFromMaster() ;     //等待其他数据帧接收完成'
									 delay_ms(10);
									 IWDG_ReloadCounter();    //喂狗
					  }
}

u32   SendTime = 0  ;
u16   SendFengYaValue = 0xFFFF ;
uint8_t Dmr400M_DataSendDataFun(uint8_t command)
{
		u8    ShuZiData[21] = {0xA5,0x5A,0x11,0xCA,0x20,0x71,0x45,0x46,0x55,0x91,0xFF,0xFF,0x12,0x34,0x56,0x28,0xF2,0xC8,0xB8,0xFF}  ; 		
		u16    i = 0 ; 	
			
//		if(command<0xD0)
//		{
//		     return 0;
//		}

		BackLinghtCount = 10 ;
		
//    JNO	= 0xFFFFFFFF ;		
		 	 
    ShuZiData[0] =  0xA5;
		ShuZiData[1] =  0x5A;
		ShuZiData[2] =  0x11;
		ShuZiData[3] =  0xCA;
			
		ShuZiData[4] =  (JNO>>24)&0xFF;	 
		ShuZiData[5] =  (JNO>>16)&0xFF; 	 
		ShuZiData[6] =  (JNO>>8)&0xFF;	     
		ShuZiData[7] =   JNO&0xFF;		     
			
		ShuZiData[8] =    0x5A	 ;	  //确认仪命令字节  		     
		ShuZiData[9] =   command ;	
	
    if(command<4)	
		{
					ShuZiData[10] = (SendFengYaValue>>8)&0xFF;	            
					ShuZiData[11] = (SendFengYaValue)&0xFF;
		}
		else
		{
					ShuZiData[10] = 0xFF;	            
					ShuZiData[11] = 0xFF;
		}

		switch(command)
		{
						case 0xD3:
				
						break ;
						case 0xD5:
							
									 GetRtcTimeData();
						
					         SendTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second ;    /*获取时间*/	
							
						break ;
						case 0xD7:
							
									 GetRtcTimeData();	
					         SendTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second ;    /*获取时间*/	
							
						break ;
						case 0xD9:
							    
      					   GetRtcTimeData();	
					         SendTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second ;    /*获取时间*/	
						
									 ShuZiData[10] =  0x00;	            
		               ShuZiData[11] =  SendCommandOprationIndex ;
						
						       SendDAD9Flag =  0xD9  ;
						
						break ;
						
						case 0xDA:	        					
									ShuZiData[10] =  0x00;	            
		              ShuZiData[11] =  SendCommandOprationIndex ;
						      SendDAD9Flag =  0xDA  ;
						break ;
						default:
							  GetRtcTimeData();
					      SendTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second ;    /*获取时间*/		
						break ;			
		}
		
	  ShuZiData[12] =  IDNUM>>16&0xFF;	 
		ShuZiData[13] =  IDNUM>>8&0xFF;
		ShuZiData[14] =  IDNUM&0xFF;
		
		if(command==0xD3)
		{
				ShuZiData[18] = (SendTime>>24)&0xFF ; 
				ShuZiData[17] = (SendTime>>16)&0xFF ; 
				ShuZiData[16] = (SendTime>>8)&0xFF ; 
				ShuZiData[15] = (SendTime)&0xFF ; 
		}
		else
		{
				ShuZiData[15] = (SendTime>>24)&0xFF ; 
				ShuZiData[16] = (SendTime>>16)&0xFF ; 
				ShuZiData[17] = (SendTime>>8)&0xFF ; 
				ShuZiData[18] = (SendTime)&0xFF ; 
		}
		

	  ShuZiData[19] = 0xFF ;      //场强
	  //计算校验码
		for(i=0;i<20;i++)
		{
				 ShuZiData[20] += ShuZiData[i] ;	
		}
		 
		ShuZiData[20] = ~ShuZiData[20] ;
		
		ShuZiData[20] +=  1;
		
		if(command>4)
		{
		   wtn_play_voice(command);
		}
		else
		{
		    wtn_play_voice_one(91);
		}
		
		printf("\r\n  Send 400M DmrData\r\n");
		
		for(i = 0; i < 21; i++)
		{		
			   printf(" %02X",ShuZiData[i]);
		}
		
		printf("\r\n");
		
		RadioChange_Zone(0,1,Dmr400M_SendCmdChanl);
		
		delay_ms(100); 
		
	  api_zzt_send_csbk_data(ShuZiData);  //发送CSBK指令
		
    Dmr400M_SendCommandFlag = 1 ;
    Dmr400M_SendCommandCount  = 0 ;
		Dmr400M_ReceCommandFlag = 0 ;
			
		delay_ms(300); 
		
		RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
		
		HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
		
}
uint8_t Dmr400M_DataSendData(uint8_t command)
{
		u8    ShuZiData[21] = {0xA5,0x5A,0x11,0xCA,0x20,0x71,0x45,0x46,0x55,0x91,0xFF,0xFF,0x12,0x34,0x56,0x28,0xF2,0xC8,0xB8,0xFF}  ; 		
		u16    i = 0 ; 	
		u8	  CommandSearchFailFlag = 0 ;
		u32   SendTime = 0  ;

		DisTemp[0] = '\0' ;
					
		if(SpkOpenFlag==0)
		{
		     AudioSpkOpen(); 
			   SpkOpenCount = 2; 
		}
		
		wtn_play_voice(command);

		BackLinghtCount = 10 ;
			
    ShuZiData[0] =  0xA5;
		ShuZiData[1] =  0x5A;
		ShuZiData[2] =  0x11;
		ShuZiData[3] =  0xCA;
			
		ShuZiData[4] =  (JNO>>24)&0xFF;	 
		ShuZiData[5] =  (JNO>>16)&0xFF; 	 
		ShuZiData[6] =  (JNO>>8)&0xFF;	     
		ShuZiData[7] =   JNO&0xFF;		     
			
		ShuZiData[8] =    0x5A	 ;	  //确认仪命令字节  		     
		ShuZiData[9] =   command ;	
		
		ShuZiData[10] =  0xFF;	            
		ShuZiData[11] =  0xFF;	
		
		if(command==0x00||command==0x01)      //0x00:关闭灯光  0x01:开启常亮
		{
			  ShuZiData[9] =   0x01 ;

				ShuZiData[10] =  0x00 ;
	      ShuZiData[11] =  command;	
			
		}	
		else if(command==0x02)  //查询状态
		{
		    command = 0x02 ;
				ShuZiData[10] =  0x01;	            
	      ShuZiData[11] =  0xFF;	
		}
		
    FengYaZhi = ShuZiData[10]<<8|ShuZiData[11] ;
		
//	  if(SendIDNUM==0xFFFFFF)
//	  {
//		    SendIDNUM = IDNUM ;	
//		}	
//		if(command==0xB5&&LinkStatus==0)
//		{
//					ShuZiData[12] =  SendIDNUM>>16&0xFF;	 
//					ShuZiData[13] =  SendIDNUM>>8&0xFF;
//					ShuZiData[14] =  SendIDNUM&0xFF;
//		
//		}
//		else
		{					
					ShuZiData[12] =  IDNUM>>16&0xFF;	 
					ShuZiData[13] =  IDNUM>>8&0xFF;
					ShuZiData[14] =  IDNUM&0xFF;
		}
		

		GetRtcTimeData();
		
		SendTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second ;    /*获取时间*/							

		ShuZiData[15] = (SendTime>>24)&0xFF ; 
		ShuZiData[16] = (SendTime>>16)&0xFF ; 
		ShuZiData[17] = (SendTime>>8)&0xFF ; 
		ShuZiData[18] = (SendTime)&0xFF ; 

		ShuZiData[19] = 0xFF ;      //场强

		if(Dmr400M_SendCmdChanl==1)
		{
				sprintf(DisTemp,"%s","404.025");
		}
		else if(Dmr400M_SendCmdChanl==2)
		{
				sprintf(DisTemp,"%s","414.025");
		}
		else if(Dmr400M_SendCmdChanl==11)
		{
				sprintf(DisTemp,"%s","408.550");
		}
		else if(Dmr400M_SendCmdChanl==12)
		{
				sprintf(DisTemp,"%s","418.550");
		}
		
    TitleTextLen = AscllCovertUniCode(DisTemp,TitleText+6) + 3 ;
		
		ShowDispJNO(JNO);	
		
		Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3;
		
		if(command==0xB5&&LinkStatus==0)
		{
			   sprintf(DisTemp,"%06X ",SendIDNUM);	
		}
		else
		{
		    sprintf(DisTemp,"%06X ",IDNUM);	
		}
		
		Item2_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;
		
	
		sprintf(DisTemp,"%s","             ") ;
		Item3_ItemLen = AscllCovertUniCode(DisTemp,Item3_Text+6) + 3 ;
		
		switch(command)
		{
						case 0x00:
								 sprintf(DisTemp,"%s","关闭灯光");		
						break ;
						case 0x01:
								 sprintf(DisTemp,"%s","开启灯光");		
						break ;
						case 0x02:
								 sprintf(DisTemp,"%s","查询状态");		
						break ;	
						case 0x24:
								 sprintf(DisTemp,"%s","待排风");		
						break ;
						case 0xB5:
								 sprintf(DisTemp,"%s","确认");
						break ;								
						case 0x91:
								 sprintf(DisTemp,"%s","风压查询");
						break ;
						case 0x93:
								 sprintf(DisTemp,"%s","自动风压");
						break ;
						case 0x83:
								sprintf(DisTemp,"%s","销号");
						break ;
						case 0x31:
								sprintf(DisTemp,"%s","风警确认");
						break ;
						case 0xB1:
								sprintf(DisTemp,"%s","电警确认");
						break ;		
						case 0xD3:
								sprintf(DisTemp,"%s","号码设置");
								ShuZiData[15] = (EditIDNUM)&0xFF ; 
								ShuZiData[16] = (EditIDNUM>>8)&0xFF ; 
								ShuZiData[17] = (EditIDNUM>>16)&0xFF ; 
								ShuZiData[18] =   0x00 ; 
						break ;
						case 0xD5:
								sprintf(DisTemp,"%s","查询模块");   //0xD6
						break ;
						case 0xD7:
								sprintf(DisTemp,"%s","查询网络");   //0xD8
						break ;
						case 0xD9:
								sprintf(DisTemp,"%s","查询参数");      //0xD9
								ShuZiData[10] =  0x00;	            
		            ShuZiData[11] =  SendCommandOprationIndex;	
						break ;
						case 0xDA:
								sprintf(DisTemp,"%s","设置参数");      //0xDA
								ShuZiData[10] =  0x00;	            
		            ShuZiData[11] =  SendCommandOprationIndex;	
						break ;
						default:	
								sprintf(DisTemp,"%02X%02X",0x5A,command);
								CommandSearchFailFlag = 1 ;			
						break ;			
		}
		
		if(CommandSearchFailFlag==1)
		{
						 Item3_ItemLen = AscllCovertUniCode(DisTemp,Item3_Text+6) + 3 ;		
		}
		else
		{		
						 Item3_ItemLen = HanZiCovertUniCode(DisTemp,Item3_Text+6) + 3 ;			
		}

		
		if(command>=0xD9&&command<=0xDA)
		{
								MenuWindows_Opration(command,SendCommandOprationIndex) ;
		}
		else
		{
								Item4_Text[0] = 0xCE;
								Item4_Text[1] = 0x98;
								Item4_Text[2] = 0x8B;
								Item4_Text[3] = 0x53;
								Item4_Text[4] = 0x1A;
								Item4_Text[5] = 0xFF;
								sprintf(DisTemp,"%04X  kPa ",FengYaZhi);
			          Item4_ItemLen = 3 ;
								Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+6) ;
		}
	
	  //计算校验码
		for(i=0;i<20;i++)
		{
				 ShuZiData[20] += ShuZiData[i] ;
		}
		 
		ShuZiData[20] = ~ShuZiData[20] ;
		
		ShuZiData[20] +=  1;
			
		printf("\r\n  Send 400M DmrData\r\n");
		
		for(i = 0; i < 21; i++)
		{		
			   printf(" %02X",ShuZiData[i]);
		}
		
		printf("\r\n\r\n");
		
		sprintf(DisTemp,"%s","               ");
		Item5_ItemLen = AscllCovertUniCode(DisTemp,Item5_Text+6) + 3 ;
		
		sprintf(DisTemp,"%s","               ");
		Item6_ItemLen = AscllCovertUniCode(DisTemp,Item6_Text+6) + 3 ;
		
		HighLightItem = 0 ;
	  Button1Value =  0x0000 ;
	  Button2Value =  0x0000 ;
		
		MenuWindowsDis(WindowID);
		
		
		for(i=0;i<2;i++)
		{
			  if(Dmr400M_SendCmdChanl!=WorkXinDaoNum)
				{
		           RadioChange_Zone(0,1,Dmr400M_SendCmdChanl) ;
				}
		}
		
		for(i=0;i<10;i++)
		{
						delay_ms(10); 
						IWDG_ReloadCounter();    //喂狗			
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
		}
		
			
	  api_zzt_send_csbk_data(ShuZiData);  //发送CSBK指令
		
    Dmr400M_SendCommandFlag = 1 ;
    Dmr400M_SendCommandCount = 0 ;
		Dmr400M_ReceCommandFlag = 0 ;
		
		if(Dmr400M_ReceCmdChanl==1)
		{
				 sprintf(DisTemp,"%s","404.025");
		}
		else if(Dmr400M_ReceCmdChanl==2)
		{
				 sprintf(DisTemp,"%s","414.025");
		}
		else if(Dmr400M_ReceCmdChanl==11)
		{
				 sprintf(DisTemp,"%s","408.550");
		}
		else if(Dmr400M_ReceCmdChanl==12)
		{
				 sprintf(DisTemp,"%s","418.550");
		}
		
    TitleTextLen = AscllCovertUniCode(DisTemp,TitleText+6) + 3 ;	
		
		
		for(i=0;i<2;i++)
		{
			  if(Dmr400M_ReceCmdChanl!=WorkXinDaoNum)
				{
		           RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
				}
		}
		
		HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
		delay_ms(1); 
		MenuWindowsDis(WindowID);
}
void Sys_Soft_Reset(void)
{  
			 printf("\r\nSystem Reset By Self!!\r\n");
			 printf("\r\nSystem Reset By Self!!!\r\n");
			 printf("\r\nSystem Reset By Self!!!\r\n");
	
			 SCB->AIRCR =0X05FA0000|(u32)0x04;    
}
void RadioSelfReset(void)
{
	     uint16_t  TempInt = 0 ;
	
	     u8    i= 0 ;
  
	     BKP_WriteBackupRegister(BKP_DR2,0xA6A6);	
	
	     TempInt = (IDNUM>>16)&0xFFFF;
	     BKP_WriteBackupRegister(BKP_DR3,TempInt);
	     TempInt = (IDNUM)&0xFFFF;
	     BKP_WriteBackupRegister(BKP_DR4,TempInt);
	
		   TempInt = LinkStatus ;
	     BKP_WriteBackupRegister(BKP_DR5,TempInt);
	
		   TempInt = (JNO>>16)&0xFFFF;
	     BKP_WriteBackupRegister(BKP_DR6,TempInt);
	     TempInt =  JNO&0xFFFF;
	     BKP_WriteBackupRegister(BKP_DR7,TempInt);
	
	
		   printf("\r\nRadioOBInit By Self!!\r\n");
			 printf("\r\nRadioOBInit By Self!!!\r\n");
			 printf("\r\nRadioOBInit By Self!!!\r\n");
	
	//   Sys_Soft_Reset();

	     RadioOBInit();
			 
			 for(i=0;i<3;i++)
			 {		
					 MenuWindows_Start(WindowID) ;
					 if(MenuWindows_DisOk==1)
					 {
								break ;
					 }
			}
				 
			 
}

uint8_t CovertKeyValue(uint8_t KeyType)
{
	
    uint8_t keyvalue = KeyType;
	
    switch(KeyType)
		{
			case KEY_SHUZI0:
				  keyvalue = KEY_SHUZI0_VALUE;
			break ;
			case KEY_SHUZI1:
				  keyvalue = KEY_SHUZI1_VALUE;
			break ;
			case KEY_SHUZI2:
				  keyvalue = KEY_SHUZI2_VALUE;
			break ;
			case KEY_SHUZI3:
				  keyvalue = KEY_SHUZI3_VALUE;
			break ;			
			case KEY_SHUZI4:
				  keyvalue = KEY_SHUZI4_VALUE;
			break ;			
			case KEY_SHUZI5:
				  keyvalue = KEY_SHUZI5_VALUE;
			break ;
			case KEY_SHUZI6:
				  keyvalue = KEY_SHUZI6_VALUE;
			break ;
			case KEY_SHUZI7:
				  keyvalue = KEY_SHUZI7_VALUE;
			break ;	
			case KEY_SHUZI8:
				  keyvalue = KEY_SHUZI8_VALUE;
			break ;	
			case KEY_SHUZI9:
				  keyvalue = KEY_SHUZI9_VALUE;
			break ;		
			case KEY_UP:
				  keyvalue = KEY_UP_VALUE;
			break ;
			case KEY_DOWN:
				  keyvalue = KEY_DOWN_VALUE;
			break ;	
			case KEY_LEFT:
				  keyvalue = KEY_LEFT_VALUE;
			break ;	
			case KEY_RIGHT:
				  keyvalue = KEY_RIGHT_VALUE;
			break ;
			case KEY_MENU:
				  keyvalue = KEY_QUEREN_VALUE;
			break ;
			case KEY_BACK:
				  keyvalue = KEY_XIAOHAO_VALUE;
			break ;
			case KEY_XING:
				  keyvalue = KEY_XING_VALUE;
			break ;
			case KEY_JING:
				  keyvalue = KEY_JING_VALUE;
			break ;
		  case KEY_Green:
				  keyvalue = KEY_CHAXUN_VALUE;
			break ;
			case PTT:
				  keyvalue = KEY_PTTKEY_VALUE;
			break ;
		  case TOP:
				  keyvalue = KEY_TOP_VALUE;
			break ;
			case FUN:
				  keyvalue = KEY_FUN_VALUE;
			break ;
			case SK1:
				  keyvalue = KEY_SK1_VALUE;
			break ;
			case SK2:
				  keyvalue = KEY_SK2_VALUE;
			break ;
			default:
				  keyvalue = KeyType ;
			break ;
		}
		
	//	KeyActiveStru.KeyType = 0xFF ;
		
    return keyvalue ;

}
/***************************************************************************
Function name   :KeyActive
Description     :GOB handle function, when radio key operated, will send a correspond
                 message to GOB, GOB will operate according as the message
Input           :receive hrnp broadcast message
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool AudioPathActive(FrameBuf_t *pHrnpCmdMsg)
{

}
/***************************************************************************
Function name   :KeyActive
Description     :GOB handle function, when radio key operated, will send a correspond
                 message to GOB, GOB will operate according as the message
Input           :receive hrnp broadcast message
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool ChangeMenuActive(FrameBuf_t *pHrnpCmdMsg)
{
	
    bool Result = TRUE;
	
	
    KeyActiveStru = HrnpGetKeyActiveType(pHrnpCmdMsg);
	
	  return Result ;
}

bool KeyActiveFuncManagement(void)
{
		uint8_t  key ;
	
	  uint8_t   i = 0 ;
	
		static uint8_t  SetJnoIndex = 0 ;

		static uint8_t  SetIdnumIndex = 0 ;
	
		static uint32_t  SetValueTemp = 0 ;
	
		static char  SetJnoTemp[10]  ;
	
		static char  SetIdnumTemp[10]  ;
	
		bool Result = TRUE;
	
		CommandIdleTimeCount = 0 ;

		printf("\r\nKeyOperate---%d, KeyValue---%d\r\n",KeyActiveStru.KeyOperate,KeyActiveStru.KeyType);
		
		if(WorkMode_LwOrCall==2)
		{
							key = CovertKeyValue(KeyActiveStru.KeyType);
							if(key==KEY_SK1_VALUE||key==KEY_SK2_VALUE||key==KEY_FUN_VALUE)
							{
							}
							else
							{
									 printf("\r\n WorkMode_LwOrCall Is Ingore,KeyOperate Is Ingore --%d\r\n",key) ;
									 return 0;				
							}
		 }
		 
		 
		 if(KeyActiveStru.KeyOperate==0x00)
	   {	

		 }
		 else if(KeyActiveStru.KeyOperate==0x01)
	   {	

		 }			 
		 else if(KeyActiveStru.KeyOperate==0x02)
	   {				
				          key = CovertKeyValue(KeyActiveStru.KeyType) ;
			 
				          printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
			 
									if(BackLinghtStatus==0||BackLinghtCount==0)
									{

													 WindowBackLightCon(1);				
									}
									else
									{		
													 BackLinghtCount = 15 ;
									}	
									
									if(key>=KEY_SHUZI0_VALUE&&key<=KEY_SHUZI9_VALUE)
									{
													 if(StartWaitQueRenFlag==WaitQueRen_SetJno)
													 {
																 if(SetJnoIndex>=8)
																 {
							                         wtn_play_voice_one(133);
																 }
																 else
																 {
																 			 wtn_play_voice(key);
																			 SetValueTemp = SetValueTemp<<4;
																			 SetValueTemp |= key ;
																			 SetJnoTemp[SetJnoIndex++] = 0x30 + key ;	 
																 }
																 
																 sprintf(DisTemp,"%01X",SetValueTemp);
																 Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3; 
																 MenuWindowsDis(0xFFFFFFFF);
                                 SpkOpenCount = 10 ;	
																 
													 }
													 else if(StartWaitQueRenFlag==WaitQueRen_SetIdnum)
													 {   
																 if(SetIdnumIndex>=6)
																 {
																	    wtn_play_voice_one(133);
																	 
																 }
																 else
																 {
																	 	 SetValueTemp = SetValueTemp<<4;
																		 SetValueTemp |= key ;
																		 SetIdnumTemp[SetIdnumIndex++] = 0x30 + key ;
																     wtn_play_voice(key);
																 }
																 sprintf(DisTemp,"%01X",SetValueTemp);
																 Item2_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;
																 MenuWindowsDis(0xFFFFFFFF);
																 SpkOpenCount = 10;				
												 	 }
													 else
													 {
													      wtn_play_voice_one(28);
													 }
									} 
									else if(key==KEY_ChannelDn_VALUE||key==KEY_ChannelUp_VALUE)
									{	
													WorkXinDaoNum =  11 ;
													Dmr400M_ReceCmdChanl = 11 ;
													Dmr400M_SendCmdChanl = 12 ;
										
													for(i=0;i<3;i++)
													{
																	Result = RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ; 
																	if(Result==TRUE)
																	{
																				 wtn_play_voice_one(28);
																				 MenuWindows_Start(WindowID) ;
                                         break ;																				 
																	} 
												  }	
									}
									else if(key==KEY_LEFT_VALUE)
									{
													if(StartWaitQueRenFlag==WaitQueRen_SetJno)
													{
														     RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
														     wtn_play_voice_one(89);        //删除音
																 if(SetJnoIndex>=1)
																 {
																        SetJnoIndex-- ;
																	      SetValueTemp = SetValueTemp>>4 ;
																	 			sprintf(DisTemp,"%01X",SetValueTemp);
																        DisTemp[SetJnoIndex] = '#';
																 }
																 Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3;
																 MenuWindowsDis(WindowID);
                                 SpkOpenCount = 10 ;																 
													 }
													 else if(StartWaitQueRenFlag==WaitQueRen_SetIdnum)
													 {
														     RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
														     wtn_play_voice_one(89);        //删除音
																 if(SetIdnumIndex>=1)
																 {
																        SetIdnumIndex-- ;
																	      SetValueTemp = SetValueTemp>>4 ;
																	 			sprintf(DisTemp,"%01X",SetValueTemp);
																        DisTemp[SetIdnumIndex] = '#';
																 }
																 Item2_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;
																 MenuWindowsDis(WindowID) ;	 
																 SpkOpenCount = 10 ;
												 	 }
													 else
													 {
														      wtn_play_voice_one(28);        //按键音
													 }
									}
									else if(key==KEY_UP_VALUE)
									{
													wtn_play_voice_one(28);              //按键音
										      if(LinkStatus==1)
													{
															AudioOut_Sel_SPK() ;
															ModemReceOK = FALSE ;
															Dmr400M_ReceCommandFlag = 0 ;
															Dmr400M_DataSendData(0x00) ;
													}
									}
									else if(key==KEY_DOWN_VALUE)
									{
										     	wtn_play_voice_one(28);              //按键音
									}
									else if(key==KEY_RIGHT_VALUE)
									{
													wtn_play_voice_one(28);              //按键音
										      if(LinkStatus==1)
												  {
																AudioOut_Sel_SPK() ;
																ModemReceOK = FALSE ;
																Dmr400M_ReceCommandFlag = 0 ;
																Dmr400M_DataSendData(0x02) ;	
												  }
									}
									else if(key==KEY_TOP_VALUE)
									{
													if(CurrentWorkFunc<4)
													{
															 CurrentWorkFunc++ ;
													}	
													else if(CurrentWorkFunc==4)
													{
															 CurrentWorkFunc = 1 ;
													}
													
													wtn_play_voice_one(28);                //按键音
													MenuWindows_Start(WindowID)	;          //显示窗口
									}

									else if(key==KEY_JING_VALUE)
									{
										         if(WorkXinDaoNum!=13)
														 {
										            RadioChange_Zone(0,1,13) ;
														 }
														wtn_play_voice(0xA0) ;
														StartWaitQueRenFlag = WaitQueRen_SetIdnum ;  
														StartWaitQueRenCount = 30;
														BackLinghtCount = 30 ;
														SetIdnumIndex = 0 ;
														SetValueTemp = 0 ;	
														HighLightItem	= 2 ;
                            LinkStatus = 0 ;										
														Button1Value =  0x0001 ;
														Button2Value =  0x0002 ;							
														if(StartWaitQueRenFlag==WaitQueRen_SetJno)
														{
																	ShowDispJNO(JNO);	
																	Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3 ; 	
														}
														sprintf(SetIdnumTemp,"%s","###### ");
														Item2_ItemLen = AscllCovertUniCode(SetIdnumTemp,Item2_Text+6) + 3 ; 
														MenuWindowsDis(0xFFFFFFFF) ;
													
														Dmr400M_ReceCommandFlag = 0 ;
														Dmr400M_SendCommandFlag = 0 ; 
									}	
									else if(key==KEY_XING_VALUE)
									{
										       	if(WorkXinDaoNum!=13)
														{
																	RadioChange_Zone(0,1,13) ;	
														}	
                            wtn_play_voice(0xA1);														 
														StartWaitQueRenFlag = WaitQueRen_SetJno ;  
														StartWaitQueRenCount = 30 ;	
														BackLinghtCount = 30 ;
														SetJnoIndex = 0 ;													
														SetValueTemp = 0 ;	
														HighLightItem	= 1 ;
										        LinkStatus = 0 ;
														Button1Value =  0x0001 ;
														Button2Value =  0x0002 ;
														sprintf(DisTemp,"%s","########   ");
														Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3;
														if(StartWaitQueRenFlag==WaitQueRen_SetIdnum)
														{
																	sprintf(SetIdnumTemp,"%06X",IDNUM);
																	Item2_ItemLen = AscllCovertUniCode(SetIdnumTemp,Item1_Text+6) + 3 ; 
														}
														MenuWindowsDis(0xFFFFFFFF);		
														Dmr400M_ReceCommandFlag = 0 ;
														Dmr400M_SendCommandFlag = 0 ; 
									}
									else if(key==KEY_CHAXUN_VALUE)
									{
													 if(LinkStatus==0)
													 {
																	 if(StartWaitQueRenFlag==0)
																	 {
																					 if(CheckJNO(JNO)==1)
																					 {
																										if(IDNUM<0xA00000)
																										{
																													 SendCommandFlag = 0xB5 ;
																										}
																										else
																										{
																														SendCommandFlag = 0x91 ;
																										}
																					 }
																					 else
																					 {
																										wtn_play_voice(134);       //  播放 "注意机车号" 声音		
																					 }
																	 }
													 }
													 else
													 {
																 if(Dmr400M_LastReceCommand==0x30)        //风压报警确认
																 {
																			SendCommandFlag = 0x31;
																			Dmr400M_LastReceCommand = 0x00 ;
																 }
																 else if(Dmr400M_LastReceCommand==0xB0)    //电量报警确认
																 {		
																			SendCommandFlag = 0xB1;
																			Dmr400M_LastReceCommand = 0x00 ;
																 }
																 else if(StartWaitQueRenFlag==WaitQueRen_PaiFeng)  //发送排风命令
																 {													
																			 StartWaitQueRenFlag = 0 ;			
																			 SendCommandFlag = 0x24;
																 }
																 else if(StartWaitQueRenFlag==WaitQueRen_XiaoHao)  //发送销号命令
																 {													
																			 StartWaitQueRenFlag = 0 ;
																			 SendCommandFlag = 0x83;
																 }																 
																 else
																 {
																				SendCommandFlag = 0x91;
																				Dmr400M_LastReceCommand = 0x00 ;
																 } 
													}																 
									}
									else if(key==KEY_QUEREN_VALUE)
									{	
										      if(StartWaitQueRenFlag!=0)
													{
																if(StartWaitQueRenFlag==WaitQueRen_SetJno)
																{
																	      if(SetJnoIndex==8)
																				{
																							 if(WorkXinDaoNum!=Dmr400M_ReceCmdChanl)
																							 {
																									   RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
																							 }
																							 
																							 wtn_play_voice_one(91) ;
																							 StartWaitQueRenFlag = 0 ;
																							 StartWaitQueRenCount = 0 ;
																							 BackLinghtCount = 15 ;
																							 SetJnoIndex = 0 ;
																							 JNO = SetValueTemp ;
																							 HighLightItem	= 0 ;	
																							 Button1Value =  0x0000 ;
																							 Button2Value =  0x0000 ;
																							 Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3 ;
																							 MenuWindows_Start(WindowID);
																					
																				}
																				else
																				{
																				       	wtn_play_voice_one(133) ;
																				}
			
																}
																if(StartWaitQueRenFlag==WaitQueRen_SetIdnum)
																{
																				if(SetIdnumIndex==6)
																				{
																							 if(WorkXinDaoNum!=Dmr400M_ReceCmdChanl)
																							 {
																									   RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
																							 }
																							 
																							 wtn_play_voice_one(91) ;
																							 StartWaitQueRenFlag = 0 ;
																							 BackLinghtCount = 15 ;
																							 SetIdnumIndex = 0 ;
																							 IDNUM = SetValueTemp ;
																							 HighLightItem = 0 ;	 
																							 Button1Value = 0x0000 ;
																							 Button2Value = 0x0000 ;		
																							 Item2_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;
																							 MenuWindows_Start(WindowID);
																							 SendIDNUM = IDNUM ;
																				}
																				else
																				{
																								wtn_play_voice_one(133) ;
																				}
																						
																}
																if(StartWaitQueRenFlag==WaitQueRen_Set5BIdnum)		
																{
																	       wtn_play_voice_one(91) ;
																				 HighLightItem = 0 ;	 
																				 Button1Value = 0x0000 ;
																				 Button2Value = 0x0000 ;	
																				 IDNUM = Rece5BIDNUM ;
																				 StartWaitQueRenFlag = 0 ;
																	       MenuWindows_Start(WindowID);
																				 
																}
												  }
													else  
													{ 
                                   wtn_play_voice_one(28);       //  播放 "按键" 声音														
													}		
									}
									else if(key==KEY_XIAOHAO_VALUE)   //返回
									{
													if(StartWaitQueRenFlag==WaitQueRen_SetJno)
													{
														     RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
														
																 wtn_play_voice_one(82); 
																 HighLightItem	= 0 ;	
																 Button1Value =  0x0000 ;
																 Button2Value =  0x0000 ;															
																 ShowDispJNO(JNO);	
														     Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3 ;	
                                 if(IDNUM>=0xA00000)		
																 {
																      sprintf(DisTemp,"      ");	
																 }
																 else
																 { 
																      sprintf(DisTemp,"%06X",IDNUM);
																 }																	
																 Item1_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;	
																 MenuWindowsDis(WindowID);
														     StartWaitQueRenFlag = 0 ;
													}
													else if(StartWaitQueRenFlag==WaitQueRen_SetIdnum)
													{
														     RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl) ;
														
																 wtn_play_voice_one(82); 
																 HighLightItem	= 0 ;	
																 Button1Value =  0x0000 ;
																 Button2Value =  0x0000 ;
														     ShowDispJNO(JNO);														
																 Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3 ;	
																 if(IDNUM>=0xA00000)		
																 {
																      sprintf(DisTemp,"      ");	
																 }
																 else
																 { 
																      sprintf(DisTemp,"%06X",IDNUM);
																 }
																 Item1_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;															
																 MenuWindowsDis(WindowID);
														     StartWaitQueRenFlag = 0 ;
													}
													else if(StartWaitQueRenFlag==WaitQueRen_PaiFeng)
													{
																 wtn_play_voice_one(82); 
																 HighLightItem	= 0 ;	
																 Button1Value =  0x0000 ;
																 Button2Value =  0x0000 ;																	
																 MenuWindowsDis(WindowID);
														     StartWaitQueRenFlag = 0 ;
													}
													else if(StartWaitQueRenFlag==WaitQueRen_XiaoHao)
													{
																 wtn_play_voice_one(82); 
																 HighLightItem	= 0 ;	
																 Button1Value =  0x0000 ;
																 Button2Value =  0x0000 ;																	
																 MenuWindowsDis(WindowID);
														     StartWaitQueRenFlag = 0 ;
													}
													else if(Dmr400M_LastReceCommand==0x5B)
													{
																 wtn_play_voice_one(82) ;
														     Dmr400M_LastReceCommand = 0 ;
																 HighLightItem	= 0 ;	
																 Button1Value =  0x0000 ;
																 Button2Value =  0x0000 ;
                                 SendIDNUM = IDNUM ;														
																 MenuWindows_Start(WindowID) ; 
													}
													else if(Dmr400M_LastReceCommand!=0x00)
													{
															   wtn_play_voice_one(82) ; //按键音	
														     MenuWindows_Start(WindowID) ; 
														     Dmr400M_LastReceCommand = 0x00 ;
													}
													else
													{

																 wtn_play_voice_one(28) ; //按键音	
													}
									}
									
									KeyActiveStru.KeyType = 0xFF ;
									KeyActiveStru.KeyOperate = 0xFF ;	
		   }
			 if(KeyActiveStru.KeyOperate==0x03&&WorkMode_LwOrCall==1)
			 {				 
				        	key = CovertKeyValue(KeyActiveStru.KeyType) ;
				          printf("\r\n KeyValue_%02X long Release! \r\n",key) ;
									PTTKeyKeepTime = 0 ;
									if(BackLinghtStatus==0)
									{
													BackLinghtStatus = 1 ;
													BackLinghtCount = 15 ;
													WindowBackLightCon(1);				
									}
									else
									{		
													BackLinghtCount = 15 ;
									}
									
									switch(key)
									{		
											  case KEY_XING_VALUE:	               //机车号设置
														wtn_play_voice_one(80);       //按键音	
														JNO = 0xAAAAAAAA ;
														Button1Value =  0x0000 ;
														Button2Value =  0x0000 ;
														LinkStatus = 0 ;
														MenuWindows_Start(WindowID) ;	
												break ;
												case KEY_JING_VALUE:	                  //主机号码设置
														wtn_play_voice_one(80);       //按键音	
														IDNUM = 0xFFFFFF ;
														Button1Value =  0x0000 ;
														Button2Value =  0x0000 ;
														LinkStatus = 0 ;
														MenuWindows_Start(WindowID) ;	
												break ;
												case KEY_QUEREN_VALUE:	              //排风请确认
												    if(LinkStatus==1&&(((JNO>>16)&0xFFFF)==0x9999))
														{			
																	StartWaitQueRenFlag = WaitQueRen_PaiFeng ;		
																	StartWaitQueRenCount = 10 ;
																	wtn_play_voice(0xA2) ;
														      sprintf(DisTemp,"%s","排风请确认  ");	
                                  Item3_ItemLen = HanZiCovertUniCode(DisTemp,Item3_Text+6) + 3 ;														
																	Button1Value =  0x0000 ;
																	Button2Value =  0x0002;
																	MenuWindowsDis(0xFFFFFFFF);
													  }
														else
														{
															    StartWaitQueRenFlag = 0 ;
															    StartWaitQueRenCount = 0 ;
														      wtn_play_voice_one(133) ;     //“请注意”音
														}
												break ;
												case KEY_XIAOHAO_VALUE:	     //销号请确认
													  if(LinkStatus==1)
														{			
																	StartWaitQueRenFlag = WaitQueRen_XiaoHao ;		
																	StartWaitQueRenCount = 10 ;
																	wtn_play_voice(0xA3) ;
															    SpkOpenCount = 3 ;		
														    	sprintf(DisTemp,"%s","销号请确认  ") ;
                                  Item3_ItemLen = HanZiCovertUniCode(DisTemp,Item3_Text+6) + 3 ;														
																	Button1Value =  0x0000 ;
																	Button2Value =  0x0002 ;
																	MenuWindowsDis(0xFFFFFFFF);
														}
														else
														{
														     		 wtn_play_voice_one(133) ;     //“请注意”音
														}

												break ;
												case KEY_CHAXUN_VALUE:	     //断连请确认
													  if(LinkStatus==1)
														{			
                                	wtn_play_voice_one(137) ;       //断连
																	LinkStatus = 0 ;
															    JNO = 0xAAAAAAAA ;
																	IDNUM = 0xFFFFFF ;
																	SendIDNUM = 0xFFFFFF ;
																	Button1Value =  0x0000 ;
																	Button2Value =  0x0000 ;
																	MenuWindows_Start(WindowID) ;	
														}
														else
														{
															    wtn_play_voice_one(80);       //按键音	
															    JNO = 0xAAAAAAAA ;
																	IDNUM = 0xFFFFFF ;
																	SendIDNUM = 0xFFFFFF ;
																	Button1Value =  0x0000 ;
																	Button2Value =  0x0000 ;
															    RadioJianTingMode = 1;
																	MenuWindows_Start(WindowID) ;	
														}
												break ;	
														
												case KEY_SHUZI0_VALUE:	            //监听模式打开关闭


												break ;
														
												case KEY_DOWN_VALUE:	     //监听模式打开关闭
													
														if(IDNUM>=0xA00000||IDNUM==0)
														{
																   wtn_play_voice(255); 
														}
														else
														{
																   wtn_play_voice(254); 
																   if(MenuWindows_PassWord()==1)
																	 {   
																				 MenuWindows_Function() ;
																	 }
																	 HighLightItem	= 0;
																	 Button1Value =  0x0000 ;
																	 Button2Value =  0x0000 ;
																	 MenuWindows_Start(WindowID) ; 
														}	 
														
												break ;
												default: 
													    BackLinghtCount = 15 ;
													    wtn_play_voice_one(28); 
												break;
									}

								 if(key==KEY_FUN_VALUE)
									{
										       	 WorkMode_LwOrCall = 1;
														 KeyFunVlaue = 0 ;
										         SpkOpenFlag = 0;
										         AudioSpkOpen();
														 wtn_play_voice_one(69);                          //列尾
										         wtn_play_voice_one(86);                          //确认仪
                             Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数											
														 MenuWindows_Start(WindowID) ;                     // OB板取消SPK独占模式	
														 RadioSpkCon(1);			                             // OB板取得SPK独占模式
														 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
														 AudioSpkMute(0);
														 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数								 
														 OBMicToRF_OutRouting_Switch(AudioToOB) ;          //模拟参数配置
														 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
														 RadioChange_Zone(0,1,Dmr400M_ReceCmdChanl);       //模拟参数配置	
														 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
														 SpkOpenFlag = 0;
										         AudioSpkOpen();
														 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	  
									}
									else if(key==KEY_SK1_VALUE)
									{
															 SpkOpenFlag = 0;
														// AudioSpkOpen();
															 if(WorkMode_LwOrCall!=2)
															 {
																		 MenuWindowsClose(WindowID) ;               // 关闭显示窗口	
															 }
															 
															 wtn_play_voice_one(57) ;                     //呼叫
															 
															 RadioChange_Zone(0,1,15) ;                   //切换信道
															 
															 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
															 for(i=0;i<30;i++)
															 {
																		Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
																		delay_ms(20) ;
																		IWDG_ReloadCounter();    //喂狗
															 }
															 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
															 RadioSpkCon(0) ;
															 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
															 OBMicToRF_OutRouting_Switch(AudioToMic) ;         //模拟参数配置
															 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
															 AudioSpkUnMute(0) ;
															 IWDG_ReloadCounter();                              //喂狗
															 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数									 
															 PTTKeyKeepTime = 60 ;
															 KeyFunVlaue = 0 ;
															 WorkMode_LwOrCall = 2 ;
						//								 RadioPttCon(1) ; 
															 IWDG_ReloadCounter();    //喂狗
															 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
															 StartCallDriverYaLinFlag = 1 ;
															 StartCallDriverYaLinCount = 0 ;	 
									}
									else if(key==KEY_SK2_VALUE)
									{
										     	 SpkOpenFlag = 0;
											//		 AudioSpkOpen();
													 if(WorkMode_LwOrCall!=2)
													 {
															 MenuWindowsClose(WindowID) ;                  // 关闭显示窗口
													 }
													 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
													 
													 RadioChange_Zone(0,1,16);                         //切换信道
													 
													 wtn_play_voice_one(57);                           //呼叫
													 
													 for(i=0;i<30;i++)
													 {
																Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
																delay_ms(20);
																IWDG_ReloadCounter();    //喂狗
													 }
													 
													 RadioSpkCon(0);
													 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;   //接收处理函数	 
													 OBMicToRF_OutRouting_Switch(AudioToMic) ;          //模拟参数配置
													 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;   //接收处理函数	
													 AudioSpkUnMute(0);
													 IWDG_ReloadCounter();    //喂狗
													 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;   //接收处理函数	
													 PTTKeyKeepTime = 60 ;
													 KeyFunVlaue = 0 ;
													 WorkMode_LwOrCall = 2 ; 
													 delay_ms(50);
													 IWDG_ReloadCounter();    //喂狗
													 Result = HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数	
													 StartCallDriverYaLinFlag = 1 ;
													 StartCallDriverYaLinCount = 0 ;
									}									
									KeyActiveStru.KeyType = 0xFF ;
									KeyActiveStru.KeyOperate = 0xFF ;
		  }
      return Result;
}
/***************************************************************************
Function name   :KeyActive
Description     :GOB handle function, when radio key operated, will send a correspond
                 message to GOB, GOB will operate according as the message
Input           :receive hrnp broadcast message
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool KeyActiveFunc(FrameBuf_t *pHrnpCmdMsg)
{		
	    bool  Result = FALSE ;
			CommandIdleTimeCount = 0 ;
			KeyActiveStru = HrnpGetKeyActiveType(pHrnpCmdMsg);
      return Result;
}

/***************************************************************************
Function name   :GetTxBroadcastInfo
Description     :get Tx broadcast information
Input           :receive hrnp broadcast message
Output          :none
Return          :Tx broadcast result
Others          :none
***************************************************************************/ 
VoiceRecord_t GetTxBroadcastInfo(FrameBuf_t *pHrnpCmdMsg)
{
    VoiceRecord_t TxBroadcastInfo;
    TxBroadcastInfo.TxProcess   = ((pHrnpCmdMsg->RcvFrame[18] << 8) + pHrnpCmdMsg->RcvFrame[17]);
    TxBroadcastInfo.TxSource    = ((pHrnpCmdMsg->RcvFrame[20] << 8) + pHrnpCmdMsg->RcvFrame[19]);
    TxBroadcastInfo.TxCallType  = ((pHrnpCmdMsg->RcvFrame[22] << 8) + pHrnpCmdMsg->RcvFrame[21]);
    TxBroadcastInfo.TxRadioID   = (pHrnpCmdMsg->RcvFrame[23] + (pHrnpCmdMsg->RcvFrame[24] << 8)
                                + (pHrnpCmdMsg->RcvFrame[25] << 16) + (pHrnpCmdMsg->RcvFrame[26] << 24));
    return TxBroadcastInfo;
}

/***************************************************************************
Function name   :GetRxBroadcastInfo
Description     :get Rx broadcast information
Input           :receive hrnp broadcast message
Output          :none
Return          :Rx broadcast result
Others          :none
***************************************************************************/ 
VoiceRecord_t GetRxBroadcastInfo(FrameBuf_t *pHrnpCmdMsg)
{
    VoiceRecord_t RxBroadcastInfo;
    RxBroadcastInfo.RxProcess   = ((pHrnpCmdMsg->RcvFrame[18] << 8) + pHrnpCmdMsg->RcvFrame[17]);
    RxBroadcastInfo.RxCallType  = ((pHrnpCmdMsg->RcvFrame[20] << 8) + pHrnpCmdMsg->RcvFrame[19]);
    RxBroadcastInfo.RxtargetID  = (pHrnpCmdMsg->RcvFrame[21] + (pHrnpCmdMsg->RcvFrame[22] << 8)
                                + (pHrnpCmdMsg->RcvFrame[23] << 16) + (pHrnpCmdMsg->RcvFrame[24] << 24));       
    RxBroadcastInfo.RxSenderID  = (pHrnpCmdMsg->RcvFrame[25] + (pHrnpCmdMsg->RcvFrame[26] << 8)
                                + (pHrnpCmdMsg->RcvFrame[27] << 16) + (pHrnpCmdMsg->RcvFrame[28] << 24));          
    return RxBroadcastInfo;
}

/***************************************************************************
Function name   :GetStatusChangeInfo
Description     :get OB status change information, when OB available or unavailable change,
                 will send a message to GOB
Input           :receive hrnp broadcast message
Output          :none
Return          :status change result
Others          :none
***************************************************************************/ 
VoiceRecord_t GetStatusChangeInfo(FrameBuf_t *pHrnpCmdMsg)
{
    VoiceRecord_t RxBroadcastInfo;
    RxBroadcastInfo.StatusChangeTargetStatus = pHrnpCmdMsg->RcvFrame[17];
    RxBroadcastInfo.StatusChangeOperation = ((pHrnpCmdMsg->RcvFrame[19]<<8)+pHrnpCmdMsg->RcvFrame[18]);
    return RxBroadcastInfo;
}

/***************************************************************************
Function name   :AudioTxRecordSwitch
Description     :callback function when receive radio broadcast transmit status
Input           :receive hrnp broadcast message
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool AudioTxRecordSwitch(FrameBuf_t *pHrnpCmdMsg)
{
    bool Result = TRUE ;
	
    VoiceRecord = GetTxBroadcastInfo(pHrnpCmdMsg);
	
    if(VOICE_TX == VoiceRecord.TxProcess)
    {
        VoiceRecord.TxRecordSwitch = 1;
    }
    else if(HANG_TIME == VoiceRecord.TxProcess)
    {
        VoiceRecord.TxRecordSwitch = 0;
    }
    else if(CALL_END == VoiceRecord.TxProcess)
    {
        VoiceRecord.TxRecordSwitch = 0;
    }
    else if(CALL_FAILED == VoiceRecord.TxProcess)
    {
        VoiceRecord.TxRecordSwitch = 0;
    }
    else if(TOT == VoiceRecord.TxProcess)
    {
        
    }
    else if(TOT_PRE_ALERT_TIME == VoiceRecord.TxProcess)
    {
         VoiceRecord.TxRecordSwitch = 0;
    }
    else
    {
          Result = FALSE;
    }
    return Result;
}


/***************************************************************************
Function name   :ReceChanelChangeProcess
Description     :callback function when receive radio broadcast channel change
Input           :receive hrnp broadcast message 
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool ReceChanelChangeProcess(FrameBuf_t *pHrnpCmdMsg)
{
    bool Result = TRUE;
	
	  u8  RadioBroastChannle = 1 ;
	
	  ChannelNotiStru = HrnpGetChannelValue(pHrnpCmdMsg);
	
	  printf("\r\nReceChanelChange Process!Type:%d--Index:%d\r\n",ChannelNotiStru.Style,ChannelNotiStru.Index[0]);
	
	  RadioBroastChannle = ChannelNotiStru.Style*10 + ChannelNotiStru.Index[0] ;
	
	  if(ChannelNotiStru.Style == 0)
		{
			
							switch(RadioBroastChannle)
							{
											case 0:
												 sprintf(DisTemp,"%s","404.025");	
												 CurrentWorkMode = WorkMode_ShuZi ;
											break ;
											case 1:
												 sprintf(DisTemp,"%s","414.025");
												 CurrentWorkMode = WorkMode_ShuZi ;
											break ;
											case 10:
												 sprintf(DisTemp,"%s","408.550");
												 CurrentWorkMode = WorkMode_ShuZi ;
											break ;
											case 11:
													sprintf(DisTemp,"%s","418.550");
													CurrentWorkMode = WorkMode_ShuZi ;
											break ;
											default:
													sprintf(DisTemp,"%s","*******  ");
											break ;
							}
							
		
		}
		else if(ChannelNotiStru.Style == 1)
		{
			      CurrentWorkMode = WorkMode_MoNi ;
						switch(RadioBroastChannle)
						{
										case 0:
											 sprintf(DisTemp,"%s","457.500");
											 WorkXinDaoNum = 3;
										break ;
										case 1:
											 sprintf(DisTemp,"%s","457.700");
												WorkXinDaoNum = 4;
												
										break ;
										case 2:
												sprintf(DisTemp,"%s","457.825");
												WorkXinDaoNum = 5;	
										break ;
										case 3:
												sprintf(DisTemp,"%s","457.975");
												WorkXinDaoNum = 6  ;
										break ;
										default:
												sprintf(DisTemp,"%s","*******  ");
										break ;
						}
		
		}
	

					 
	  if(WorkMode_LwOrCall==1)
		{
					 TitleTextLen = AscllCovertUniCode(DisTemp,TitleText+6) + 3 ;	
					 if(RadioJianTingMode==1)
					 {
								sprintf(DisTemp,"监听");
								TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+20) ;
					 }			
					  MenuWindowsDis(WindowID); 
		}
		
    return Result;

}
/***************************************************************************
Function name   :AudioRxRecordSwitch
Description     :callback function when receive radio broadcast receive status
Input           :receive hrnp broadcast message 
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool AudioRxRecordSwitch(FrameBuf_t *pHrnpCmdMsg)
{
    bool Result = TRUE;
    VoiceRecord = GetRxBroadcastInfo(pHrnpCmdMsg);
    if(VOICE_RX == VoiceRecord.RxProcess)
    {
        VoiceRecord.RxRecordSwitch = 1;
    }
    else if(HANG_TIME == VoiceRecord.RxProcess)
    {
        VoiceRecord.RxRecordSwitch = 0;
    }
    else if(CALL_END == VoiceRecord.RxProcess)
    {
        VoiceRecord.RxRecordSwitch = 0;
    }
    else
    {
        Result = FALSE;
    }
    return Result;

}

/***************************************************************************
Function name   :StatusChangeNotiProcess
Description     :handle function when receive radio status change message
Input           :receive hrnp message
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool StatusChangeNotiProcess(FrameBuf_t *pHrnpCmdMsg)
{
    VoiceRecord = GetStatusChangeInfo(pHrnpCmdMsg);
    /* handle recording start or stop */
//    if(VoiceRecord.StatusChangeTargetStatus == SPK_STATUS_CHANGE)
//    {
//        if(VoiceRecord.StatusChangeOperation == SPKON)
//        {
//            /* start record rx voice */ 
//            RecordingStart(); 
//        }
//        else
//        {
//            /* stop record rx voice */
//            RecordingStop(); 
//        }
//    }
    /* handle volume change operation */
//   else 
		if(VoiceRecord.StatusChangeTargetStatus == VOLUME_CHANGE)
    {
        McuTimerForVolumeChange(ON, 100);
        ChangeVolumeFlag++;
        RadioVolumeValue = VoiceRecord.StatusChangeOperation*46/32-34; 
    }
    /* get OB status in current channel */
    else if(VoiceRecord.StatusChangeTargetStatus == OB_STATUS_CHANGE)
    {
        if(VoiceRecord.StatusChangeOperation == 1)
        {
            OBStatusInCurCh = 1;
        }
        else
        {
            OBStatusInCurCh = 0;
        }
    }
    else
    {
    }
    return TRUE;
}

// 十进制转BCD码
uint8_t  hex_to_dec(uint8_t hex)
{
	uint8_t DEC_H, DEC_L;
	uint8_t DEC;          
	if((hex >= 0)&&(hex <= 99))
	{
				DEC_H = hex / 10; 
				DEC_L = hex % 10; 
				DEC = (DEC_H<<4)|DEC_L;
  }
	return DEC;
}
/* DMR 数据整理后的数据信息 */
typedef struct t_lw_data_info
{
	uint8_t locom[4]; 			/* 0123 */
	uint8_t command[2];			/* 45 */
	uint8_t wind_pr[2];			/* 67 */
	uint8_t id_num[3];			/* 8 9 10*/	
	uint8_t time[4];				/* 11 12 13 14 */

	uint8_t rssi;						/*  16 */
	uint8_t dmr_chan;				/*  15 */
	
	unsigned int data_time ;
	unsigned char buf[5];
	
}t_def_lw_data_info;

/* 中转台发送CSBK数据帧格式 */
typedef struct t_zzt_tx_csbk_format
{
	/* 帧头 */
	uint8_t head;
	/* 数据类型 */
	uint8_t opcode[2];
	/* 数据长度 */
	uint8_t data_len[2];
	/* 时隙 */
	uint8_t timeslot;
	/* 数据 */
	uint8_t data[10];
	
}t_def_zzt_tx_csbk_format ;



/* 检验和 取反加1 */
uint8_t cal_check_0_sum(uint8_t* p, uint16_t len)
{
	uint16_t i;
	uint8_t	sum = 0;

	for (i = 0; i < len; i++)
	{
		sum = p[i] + sum;
	}
	sum = (~sum) + 1;
	return sum;
}

/****************************************************************************
* 名		称: void dmr_400m_send_data_zpack(struct t_lw_data_info* t_p, uint8_t* sendbuf)
* 功		能: 发送列尾数据 --400MHz
* 入口参数: 无
* 出口参数: 无
* 说		明:
****************************************************************************/
void dmr_400m_send_data_zpack(struct t_lw_data_info* t_p, uint8_t* sendbuf)
{
	
				const uint8_t send_buf_data[] = { 0xA5,0x5A,0x11,0xCA }; //10 11 12 16 14 15 
				uint8_t  ptr = 0 ;
				
		//	struct ffsk_tail_info ffsk_data;
				memcpy(sendbuf, send_buf_data, sizeof(send_buf_data));

				ptr = sizeof(send_buf_data);
				memcpy(sendbuf + ptr, t_p->locom, 4);
				ptr += 4;
				sendbuf[ptr++] = t_p->command[0];
				sendbuf[ptr++] = t_p->command[1];
				
				sendbuf[ptr++] = t_p->wind_pr[0];
				sendbuf[ptr++] = t_p->wind_pr[1];
				
				memcpy(sendbuf + ptr, t_p->id_num, 3);
				ptr += 3;
				memcpy(sendbuf + ptr, t_p->time, 4);	
				ptr += 4;
				sendbuf[ptr++] = t_p->rssi;
				
				sendbuf[ptr] = cal_check_0_sum(sendbuf, ptr) ;
				
				memcpy(Dmr400MReceFrame, sendbuf, 21) ;     //将接收到的数据转移到其他缓存

				Dmr400M_DataReceFlag = 1 ;		
				
}
bool AnalysisCSBKToLieWei_DataFrame(uint8_t *buf1 ,uint8_t *buf2)
{	
	
				uint8_t c_num = 0;
	
				uint8_t send_buf[40] ={0x00};
				
				t_def_lw_data_info t_c;
				
				uint16_t tomp =0;
				
				
				/* 机车号千万位，百万位 */
				tomp =(((buf1[3]&0x07)<<4)| (buf1[4] >>4))&0x7F;
				if(tomp >99)
				{
					t_c.locom[0] =0xFF;
				}
				else
				{
					t_c.locom[0] =hex_to_dec(tomp);
				}
		
				/*机车号 十万，万 */
				tomp =((uint16_t)(buf1[4]&0x0F)<<6) | (buf1[5] >>2);
				if(tomp >999)
				{
					t_c.locom[1] =0xFF;
				}
				else
				{
					t_c.locom[1] =hex_to_dec(tomp/10);
					c_num =(tomp%10)*10;
				}

			  /*机车号 千，百，十，个 */
				tomp =((uint16_t)(buf1[5] &0x03)<<8) | (buf1[6]);
				if(tomp >999)
				{
					t_c.locom[2] =0xFF;
					t_c.locom[3] =0xFF;
				}
				else
				{
					c_num =c_num+tomp/100;
					t_c.locom[2] =hex_to_dec(c_num);
					t_c.locom[3] =hex_to_dec(tomp%100);
				}
				
				/* 指令高字节 */
				t_c.command[0] =buf1[7];
				/* 指令低字节 */
				t_c.command[1] =buf1[8];
				
				/* 风压 */
				tomp =((uint16_t)buf1[9]<<2) | ((buf2[3] &0x60)>>5);
				if(tomp >999)
				{
					t_c.wind_pr[0] =t_c.wind_pr[1] =0xFF;
				}
				else
				{
					t_c.wind_pr[0] =hex_to_dec(tomp/100);
					t_c.wind_pr[1] =hex_to_dec(tomp%100);
				}
				
				/* 列尾主机ID号  十万，万  千 -- 10bit */
				tomp =(uint16_t )(buf2[3] &0x1F)<<5 |(buf2[4] >>3);   //ID 最高位错误  	tomp =(uint16_t )(buf2[3] &0x0F)<<5 |(buf2[4] >>3);
				if(tomp >999)
				{
							t_c.id_num[0] =0xFF;
				}
				else
				{
							t_c.id_num[0] =hex_to_dec(tomp/10);
							c_num =(tomp%10)*10;
				}
				
				/* 列尾主机ID号，百，十，个  -- 10bit*/
				tomp =(uint16_t )(buf2[4] &0x07)<<7 |(buf2[5] >>1);
				if(tomp >999)
				{
							t_c.id_num[1] =0xFF;
							t_c.id_num[2] =0xFF;
				}
				else
				{
							c_num =c_num+(tomp /100);
							t_c.id_num[1] =hex_to_dec(c_num);
							t_c.id_num[2] =hex_to_dec(tomp%100);
				}
				
				/* 时间 */
				t_c.time[0] =(buf2[5]&0x01)<<7 | (buf2[6]>>1);
				t_c.time[1] =(buf2[6]&0x01)<<7 | (buf2[7]>>1);
				t_c.time[2] =(buf2[7]&0x01)<<7 | (buf2[8]>>1);
				t_c.time[3] =(buf2[8]&0x01)<<7 | (buf2[9]>>1);
				
				t_c.rssi = (buf2[11]) ;   
						
 			  dmr_400m_send_data_zpack(&t_c,send_buf) ;

}


/***************************************************************************
Function name   :ReceCsbkDataProcess
Description     :handle function when receive radio csbk  message
Input           :receive hrnp message
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool ReceCsbkDataProcess(FrameBuf_t *pHrnpCmdMsg)
{

	  uint8_t frameindex = 0 ;
	  static  uint8_t  csbk_Firstframe[12] ={0x00};
    static  uint8_t  csbk_Secondframe[12] ={0x00};
		
		frameindex = pHrnpCmdMsg->RcvFrame[20]&0x80 ;              //判断是第几帧
		
	//	printf("\r\n ReceCsbkDataProcess---%02X %02X\r\n",frameindex);

//  if(cmdh==0x5D||cmdh==0x55||cmdh==0x5B)                     //命令高字节设备类型为机车台或者主机
		if(frameindex==0x00)                                       //第一帧
		{  
//			     printf("\r\n Receive First Frame \r\n");
					 memcpy(csbk_Firstframe,&pHrnpCmdMsg->RcvFrame[17],12);
					 gTimerSwitchForReceCsbk = ON ;
					 gTimerClockForReceCsbk = 1000 ;
					 CSBKFrameIndex = 2 ;	
    }
//  else if(CSBKFrameIndex==2&&ON==gTimerSwitchForReceCsbk)      //1S内接收到的可以组包
		else if(frameindex==0x80&&ON==gTimerSwitchForReceCsbk)       //1S内接收到的可以组包
		{	
//			     printf("\r\n Receive Two Frame \r\n");
					 memcpy(csbk_Secondframe,&pHrnpCmdMsg->RcvFrame[17],12); 
					 AnalysisCSBKToLieWei_DataFrame(csbk_Firstframe,csbk_Secondframe);  
					 CSBKFrameIndex = 0 ;
			     gTimerSwitchForReceCsbk = OFF ;
    }
		else
		{
			     printf("\r\n Receive Quit \r\n");
		       CSBKFrameIndex = 0 ;		
           gTimerSwitchForReceCsbk = OFF ;		
		}
    return TRUE;
}
/*数值转换 0x10 - 10 */
uint8_t hex_bcd(uint8_t num)
{
	
		uint8_t result =0;
		
		result =(num >>4)*10 + (num &0x0F);
		
		return result;
}
/****************************************************************************
* 名称: void api_csbk_frist_frame_pack(uint8_t *buf,struct t_lw_data_info *p)
* 功能:	一帧列尾数据要转化成2帧CSBK数据，此函数为第一帧数据打包
* 入口参数: 
* 说明:
***************************************************************************/
void api_csbk_frist_frame_pack(uint8_t *buf,struct t_lw_data_info *p)
{
			uint16_t temp =0;
			uint8_t temp_buf[10] ={0x00};
			
			temp_buf[0] =0x10 ;
			temp_buf[1] =0x20 ;
			
			temp_buf[2] =0xFF ;
			
			/* 如果机车号参数为0xFF */
			if(p->locom[0] ==0xFF || p->locom[1] ==0xFF || p->locom[2] ==0xFF ||p->locom[3] ==0xFF)
			{
						temp_buf[3] = 0x0F;
						temp_buf[4] = 0xFF;
						temp_buf[5] = 0xFF;
						temp_buf[6] = 0xFF;
			}
			else
			{
					temp_buf[3] =(1<<3) |(hex_bcd(p->locom[0])&0x70) >>4;
					
					temp =(p->locom[1] >>4)*100 +(p->locom[1]&0x0F)*10 + (p->locom[2]>>4);

					temp_buf[4] =hex_bcd(p->locom[0])<<4 |(temp >>6);
					
					temp_buf[5] =(uint8_t)temp <<2 ;
					
					temp =(p->locom[2] &0x0F)*100 + (p->locom[3]>>4)*10 +(p->locom[3] &0x0F);
					
					temp_buf[5] =temp_buf[5] |(temp >>8);
					
					temp_buf[6] =(uint8_t)temp;
			}
			
			temp_buf[7] = p->command[0];
			temp_buf[8] = p->command[1];
			
			/* 如果风压参数为0xFF  */
			if(p->wind_pr[0]==0xFF || p->wind_pr[1] ==0xFF)
			{
					 temp_buf[9] =0xFF;
			}
			else
			{
					temp =(p->wind_pr[0]*100) + ((p->wind_pr[1]>>4)*10) +(p->wind_pr[1]&0x0F);
					temp_buf[9] =temp >>2;
			}
			
			memcpy(buf,temp_buf,10);
			
}


/****************************************************************************
* 名称: void api_csbk_frist_frame_pack(uint8_t *buf,struct t_lw_data_info *p)
* 功能:	一帧列尾数据要转化成2帧CSBK数据，此函数为第二帧数据打包
* 入口参数: 
* 说明:
***************************************************************************/
void api_csbk_two_frame_pack(uint8_t *buf,struct t_lw_data_info *p)
{
	uint16_t tecp =0;
	uint8_t temp_buf[10] ={0x00};

	temp_buf[0] =0x90;
	temp_buf[1] =0x20;
	
	
	temp_buf[2] =0xFF;
	
	if(p->wind_pr[0]==0xFF || p->wind_pr[1] ==0xFF)
	{
		tecp =0xFFFF;
	}
	else
	{
		tecp =(p->wind_pr[0]*100) + ((p->wind_pr[1]>>4)*10) +(p->wind_pr[1]&0x0F);
	}		

	temp_buf[3] =(1<<7) |((tecp &0x03)<<5) ;
	
	if(p->id_num[0] ==0xFF || p->id_num[1] ==0xFF|| p->id_num[2] ==0xFF)
	{
		tecp =0xFFFF;
	}
	else
	{
		tecp =(p->id_num[0] >> 4)*100 +(p->id_num[0] &0x0F)*10 +(p->id_num[1]>>4);
	}
	
	temp_buf[3] =temp_buf[3] |tecp>>5;
	
	temp_buf[4] =tecp<<3;
	
	
	if(p->id_num[0] ==0xFF || p->id_num[1] ==0xFF|| p->id_num[2] ==0xFF)
	{
		tecp =0xFFFF;
	}
	else
	{
		tecp =(p->id_num[1] &0x0F)*100 +(p->id_num[2] >>4)*10 +(p->id_num[2] &0x0F);
	}
	
	temp_buf[4] =temp_buf[4] |tecp>>7;
	
	temp_buf[5] =tecp<<1;
	
	temp_buf[5] =temp_buf[5] |(p->time[0] >>7);
	
	temp_buf[6] =(p->time[0] <<1) | (p->time[1] >>7);

  temp_buf[7] =(p->time[1] <<1) | (p->time[2] >>7);
	
	temp_buf[8] =(p->time[2] <<1) | (p->time[3] >>7);

	temp_buf[9] =(p->time[3] <<1);
	
	if(WorkXinDaoNum==11||WorkXinDaoNum==12||WorkXinDaoNum==14)
	{
	     temp_buf[9]&=0xFE ;
	}
	else
	{
	     temp_buf[9]|=0x01 ;
	}
	memcpy(buf,temp_buf,10);
	
}
struct csbk_format
{
	uint8_t locom_num[4];
	uint8_t command_h;
	uint8_t command_l;
	uint8_t wind_pr[2];
	uint8_t tail_id[3];
	uint8_t time[4];
	uint8_t rssi;
	uint8_t check_sum;
};
/****************************************************************************
* 名		称: void recive_csbk_to_user_data(struct csbk_format * p_csbk, struct t_lw_data_info* p_user_data)
* 功		能:	CSBK  转化为列尾数据
* 入口参数: 
* 说		明:
****************************************************************************/
void recive_csbk_to_user_data(struct csbk_format * p_csbk, struct t_lw_data_info* p_user_data)
{
	/* command */
	p_user_data->command[0] = p_csbk->command_h;
	p_user_data->command[1] = p_csbk->command_l;

	/* id num */
	memcpy(p_user_data->id_num, p_csbk->tail_id, 3);
	/*locom num*/
	memcpy(p_user_data->locom, p_csbk->locom_num, 4);

	memcpy(p_user_data->wind_pr, p_csbk->wind_pr, 2);

	/* time */
	memcpy(p_user_data->time, p_csbk->time, 4);

	p_user_data->rssi = p_csbk->rssi;
	/* dmr tx or rx channel */
	p_user_data->dmr_chan = 1;
	
	//printf("\n ------ %d %d  %02X",p_user_data->dmr_chan ,p_ffsk->dmr_chan ,p_user_data->time[0]  );
}
/****************************************************************************
* 名		称: uint8_t check_hrcp(uint8_t *buf,uint8_t len)
* 功		能:	hrcp校验
* 入口参数: buf: 需要计算校验的数组   len:数组长度
* 说		明:
****************************************************************************/
uint8_t check_hrcp(uint8_t *buf,uint8_t len)
{
	uint8_t result =0;
	uint8_t i=0;
	
	for( i=0; i<len; i++)
	{
		result =result +buf[i];
	}
	
	result =~(result)+0x33;
	return result;
}
/****************************************************************************
* 名		称: uint16_t check_hrnp(uint8_t *data,uint16_t len)
* 功		能:	hrnp校验
* 入口参数: buf: 需要计算校验的数组   len:数组长度
* 说		明:
****************************************************************************/
uint16_t check_hrnp(uint8_t *data,uint16_t len)
{
	long sum =0;
	uint16_t temp =0;
	uint8_t i=0 ;
	
	if(len %2 ==0)
	{
		for(i=0; i< len; i=i+2)
		{
			sum =sum + (data[i]<<8 |data[i+1]);
			if(sum >>16) 
			{
				sum = (sum & 0xffff) + (sum >> 16);
			}
		}
	}
	else
	{
		for(i=0;i<len-1;i=i+2)
		{
			sum =sum +(data[i]<<8 |data[i+1]);
			if(sum >>16) 
			{
				sum = (sum & 0xffff) + (sum >> 16);
			}
		}
		temp =data[len -1]<<8;
		sum =sum +temp;
	}
	
	if(sum >>16) 
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	
	return (uint16_t )~sum;
}
uint32_t pn_num ; 
extern  uint16_t gHrnpTxDataPN ;
/****************************************************************************
* 名		称: void zzt_tx_csbk_pack(struct zzt_tx_csbk_format* t_p, uint8_t* sendbuf)
* 功		能:	打包CSBK 的发送数据 
* 入口参数: 
* 说		明:
****************************************************************************/
void zzt_tx_csbk_pack(const struct t_zzt_tx_csbk_format* t_p, uint8_t* sendbuf)
{
	
//	uint8_t  ptr = 0;

//	uint8_t  hrc_buf[18]={0x00};
//		
//	uint16_t crc_hrnp =0;
//	
//	uint8_t  temp_buf[30] = {0x7e,0x03,0x00,0x00,0x20,0x10,0x00,0x01,0x00,0x1e
//												   ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
//											  	 ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//	
// // pn_num =pn_num+1;		

//  //gHrnpTxDataPN												
//	temp_buf[6] =	gHrnpTxDataPN >>8;	
//	temp_buf[7] =	gHrnpTxDataPN &0xFF;	

//  gHrnpTxDataPN++ ;												
//	
//	hrc_buf[ptr++] =t_p->head;
//	
//	hrc_buf[ptr++] =t_p->opcode[0];
//	hrc_buf[ptr++] =t_p->opcode[1];
//	
//	hrc_buf[ptr++] =t_p->data_len[0];
//	hrc_buf[ptr++] =t_p->data_len[1];
//	
//	hrc_buf[ptr++] =t_p->timeslot;
//	
//	memcpy(hrc_buf+ptr,t_p->data,t_p->data_len[1]-1);
//	
//	ptr += t_p->data_len[1] -1;
//	
//	/*  计算CRC校验 -HRCP */
//	hrc_buf[ptr++] =check_hrcp(hrc_buf+1,t_p->data_len[1]+4);
//	hrc_buf[ptr++] =0x03;
//	
//  /* 数据段  */
//	memcpy(temp_buf+12,hrc_buf,18);
//															
//	/* 计算CRC校验 -HRNP */	
//	crc_hrnp =check_hrnp(temp_buf,temp_buf[9]);	
//	temp_buf[10] =crc_hrnp >>8 &0xFF;	
//	temp_buf[11] =crc_hrnp&0xFF;
//	
//	memcpy(sendbuf,temp_buf,30);
}

/****************************************************************************
* 名		称: void api_zzt_sen_csbk_data(uint8_t *buf)
* 功		能:	外部端口发送列尾数据 -转化为CSBK数据到中转台发送
* 入口参数: 
* 说		明:
***************************************************************************/
uint8_t z_send_buf[30] ={0x00};

extern void USART1_SendDataBuf(u8* buf, u16 len);

void api_zzt_send_csbk_data(uint8_t *buf)
{
			struct csbk_format *p_csbk_rx = NULL ; 
	
			struct t_lw_data_info t_rx_lw_info = {0} ;
			
			uint8_t send_buf_frist[10] = {0x00} ;
			
			uint8_t send_buf_two[10] = {0x00} ;
			
			p_csbk_rx = (struct csbk_format*)(buf + 4) ;	
			
			recive_csbk_to_user_data(p_csbk_rx, &t_rx_lw_info) ;	
			
			api_csbk_frist_frame_pack(send_buf_frist,&t_rx_lw_info) ;  //第一包数据
			
			api_csbk_two_frame_pack(send_buf_two,&t_rx_lw_info) ;      //第二包数据
			
			RadioSendCsbkFunction(send_buf_frist,send_buf_two) ;
					
}








/***************************************************************************
Function name   :RegisterHdpepCmdRespondFunc
Description     :register the call back function of commands
Input           :none 
Output          :none
Return          :TRUE
Others          :none
***************************************************************************/ 
void RegisterHdpepCmdRespondFunc(void)
{
  	/* master command register */
    RegisterHrccSys(BROADCAST_KEY_OPERATION, KeyActiveFunc);
	
//  RegisterHrccSys(BROADCAST_TRANSMIT_STATUS, AudioTxRecordSwitch);
	
//  RegisterHrccSys(BROADCAST_RECEIVE_STATUS, AudioRxRecordSwitch);
	
//  RegisterHrccSys(STATUS_CHANGE_NOTI, StatusChangeNotiProcess);
	
    RegisterHrccSys(BROADCAST_CSBK_RECEDATA, ReceCsbkDataProcess);
	
//	RegisterHrccSys(BROADCAST_CHANNEL_CHANGE, ReceChanelChangeProcess);
	
}

/***************************************************************************
Function name   :MasterRecordCfg
Description     :config the radio for recording voice
Input           :none 
Output          :none
Return          :TRUE
Others          :none
***************************************************************************/ 
bool MasterRecordCfg(void)
{
    SendResult_t CheckResult;
	
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
//	  uint16_t  OB_REGISTER_OVER = 0xFFFF ;

    uint16_t MasterRecordCfgItem       = OB_REGISTER_REQUEST ;	

    uint8_t KeyOperationNotifyRq_SK[]  = {27,0x01,0x02,0x02,0x02,0x03,0x01,0x04,0x02,0x05,0x02,0x34,0x02,0x06,0x02,0x07, 0x02, 0x08, 0x02, 0x09, 0x02,0x0A, 0x02, 0x0B, 0x02, 0x0C, 0x02, 0x0D, 0x02, 0x0E, 0x02, 0x0F, 0x02, 0x10, 0x02,0x11, 0x02, 0x12, 0x02, 0x13, 0x02, 0x14, 0x02,0x15, 0x02, 0x16, 0x02, 0x17,0x02,0x18,0x02,0x22,0x02,0x23,0x02};

		
		

    uint8_t StatusChangeNotifyReq[]    = {0x03, 0x01, 0x01, 0x04, 0x01, 0x0F, 0x01};


    uint8_t InputAudioPathRq[]         = {0x03};
		
		uint8_t ChannleChageRq[]           = {0x01,0x00,0x00,0x00};

    uint8_t FunctionStatusCheck[]      = {0x02}; 

    uint8_t OBRegisterRq[]             = {0x00, 0x00, 0x00, 0x00, 0x00};
   
    uint8_t OBRegisterCsbkRq[]         = {0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
   
    while(1)
    {
								switch(MasterRecordCfgItem)
								{
															case OB_REGISTER_REQUEST:
																
																				CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, OB_REGISTER_REQUEST, OB_REGISTER_REQUEST_REPLY, OBRegisterRq, sizeof(OBRegisterRq), HrnpReplyMsg);
																		
																				if(SEND_RESULT_FAILED == CheckResult)
																				{
																						SystemStatus = STATUS_SYSTEM_FREE;
																				}
																				else if(SEND_RESULT_SUCCESSFUL == CheckResult)
																				{
																						SystemStatus = STATUS_SYSTEM_FREE;
																						MasterRecordCfgItem = KEY_OPERATION_NOTI;
																						printf("\r\nOB_Register_Request Function Set Successed!\r\n");
																				}
																				else
																				{
																					
																				}
															break;
															case FUNCTION_STATUS_CHK_REQ:
																
																	CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, FUNCTION_STATUS_CHK_REQ, FUNCTION_STATUS_CHK_REQ_REPLY, FunctionStatusCheck, sizeof(FunctionStatusCheck), HrnpReplyMsg);
																	if(SEND_RESULT_FAILED == CheckResult)
																	{
																			 SystemStatus = STATUS_SYSTEM_FREE;
																	}
																	else if(SEND_RESULT_SUCCESSFUL == CheckResult)
																	{
																				SystemStatus = STATUS_SYSTEM_FREE;
																				OBStatusInCurCh = HrnpReplyMsg->RcvFrame[19];
																				MasterRecordCfgItem = INPUT_AUDIO_PATH;
																	}
																	else
																	{
																	}
																	break;

															case INPUT_AUDIO_PATH:
																
																	CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, INPUT_AUDIO_PATH, INPUT_AUDIO_PATH_REPLY, InputAudioPathRq, sizeof(InputAudioPathRq), HrnpReplyMsg);
																	if(SEND_RESULT_FAILED == CheckResult)
																	{
																			SystemStatus = STATUS_SYSTEM_FREE;
																	}
																	else if(SEND_RESULT_SUCCESSFUL == CheckResult)
																	{
																			SystemStatus = STATUS_SYSTEM_FREE;
																			MasterRecordCfgItem = STATUS_CHANGE_NOTI_REQ;
																	}
																	else
																	{
																		
																	}
																	break;
																
															case STATUS_CHANGE_NOTI_REQ:
																
																	CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, STATUS_CHANGE_NOTI_REQ, STATUS_CHANGE_NOTI_REQ_REPLY, StatusChangeNotifyReq, sizeof(StatusChangeNotifyReq), HrnpReplyMsg);
															
																	if(SEND_RESULT_FAILED == CheckResult)
																	{
																			SystemStatus = STATUS_SYSTEM_FREE ;
																	}
																	else if(SEND_RESULT_SUCCESSFUL == CheckResult)
																	{
																			SystemStatus = STATUS_SYSTEM_FREE ;
																			MasterRecordCfgItem = KEY_OPERATION_NOTI ;
																	}
																	else
																	{  
																	}
																	break;
															case KEY_OPERATION_NOTI:
																
																		CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, KEY_OPERATION_NOTI, KEY_OPERATION_NOTI_REPLY, KeyOperationNotifyRq_SK, sizeof(KeyOperationNotifyRq_SK), HrnpReplyMsg);
																		if(SEND_RESULT_FAILED == CheckResult)
																		{
																						SystemStatus = STATUS_SYSTEM_FREE;
																		}
																		else if(SEND_RESULT_SUCCESSFUL == CheckResult)
																		{
																						SystemStatus = STATUS_SYSTEM_FREE ;
																						printf("\r\nKey_Opration_Brocast Function Set Successed!\r\n") ;
																						MasterRecordCfgItem = CHANNEL_CHANGE_REQUST ;
																		}
																		else
																		{
																			
																		}
																	break; 
																		
														case CHANNEL_CHANGE_REQUST:
																
																		CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, CHANNEL_CHANGE_REQUST, CHANNEL_CHANGE_REPLY, ChannleChageRq, sizeof(ChannleChageRq), HrnpReplyMsg);
																		
																		if(SEND_RESULT_FAILED == CheckResult)
																		{
																					SystemStatus = STATUS_SYSTEM_FREE;
																		}
																		else if(SEND_RESULT_SUCCESSFUL == CheckResult)
																		{
																					SystemStatus = STATUS_SYSTEM_FREE;
																					MasterRecordCfgItem = RADIO_CSBK_REQUEST ;
																					printf("\r\nChannel_Change_Brocast Function Set Successed!\r\n");
																		}
																		else
																		{
																			
																		}
															break; 				
															case RADIO_CSBK_REQUEST:
																		
																		CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, RADIO_CSBK_REQUEST, RADIO_CSBK_REQUEST_REPLY, OBRegisterCsbkRq, sizeof(OBRegisterCsbkRq), HrnpReplyMsg);
																		if(SEND_RESULT_FAILED == CheckResult)
																		{
																					SystemStatus = STATUS_SYSTEM_FREE;
																		}
																		else if(SEND_RESULT_SUCCESSFUL == CheckResult)
																		{
																					SystemStatus = STATUS_SYSTEM_FREE;
																					MasterRecordCfgItem = OB_REGISTER_OVER;	 
																					printf("\r\n CSBK Function Regester OK !\r\n");
																		}
															break; 
                              case OB_REGISTER_OVER:
																
																			 return CheckResult ;
															
	                            break; 																 						
															default:				
																	 return CheckResult;
															break;		
								}
								/* check the conncet state */
								if(OFF == gConnectStateFlag)
								{
												SystemStatus = STATUS_SYSTEM_FREE;
												return FALSE;
								}
    }
}
extern void Sys_Soft_Reset(void);
extern void RegisterHdpepCmdRespondFunc(void);
/***************************************************************************
Function name   :RadioOBInit
Description     :record function init
Input           :none
Output          :none
Return          :TRUE or FALSE
Others          :none
***************************************************************************/ 
bool RadioOBInit(void)
{
	
    SendResult_t CheckResult ;
	  uint8_t  SendConnectCount = 0;
	  printf("\r\n OB Board Connect To Radio......\r\n");	
	  gConnectStateFlag = OFF ;
    RegisterHdpepCmdRespondFunc();
	  SendConnectCount = 0 ;
	
    while(1)
    {	
			    /* connect state check */
					 if(OFF == gConnectStateFlag)
					 {
									CheckResult = HrnpSlaveConnectToMaster() ;    //连接对讲机
									if(SEND_RESULT_FAILED == CheckResult)
									{
													 SystemStatus = STATUS_SYSTEM_FREE ;
													 SendConnectCount++ ;
													 if(SendConnectCount>15)
													 {
															   Sys_Soft_Reset();
													 }
									}
									else if(SEND_RESULT_SUCCESSFUL == CheckResult)
									{
												   SystemStatus = STATUS_SYSTEM_FREE;
									}
					 }
					 else
					 {
													 MasterRecordCfg() ;                          //注册需要的功能 
													 return TRUE;
					 }
    }
}

/***************************************************************************
Function name   :PowerUpBackUpRecordInfo
Description     :when radio power up, back up the recording information before
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/
void PowerUpBackUpRecordInfo(void)
{
//    ReadCurPageSPARecDataAddr(SaveAddrInfoAddr,&RecDataAddrInfo);
//    if(RecDataAddrInfo.RecDataLastPageID == RECDATALASTPAGEID)
//    {
//        SaveDataEndAddr = RecDataAddrInfo.EachRecDataStopAddr;
//        SaveDataAddr    = SaveDataEndAddr;
//        RePlayDataAddr  = SaveDataEndAddr;
//        DrvNandFlashAddressIncrement(FLASH_READ,&SaveDataAddr);
//        ReadCurPageSPARecDataAddr(SaveDataAddr,&PwrupPageSPAInfo);
//        if((PwrupPageSPAInfo.RecDataPageID == RECDATAPAGEID)&& \
//           (PwrupPageSPAInfo.RecDataLastPageID == 0xffff))   //判断是否掉电
//        {
//            ReadDataAddr = SaveDataAddr;
//            do
//            {
//                DrvNandFlashAddressIncrement(FLASH_READ,&SaveDataAddr);
//                ReadCurPageSPARecDataAddr(SaveDataAddr,&PwrupPageSPAInfo);
//            }
//            while((PwrupPageSPAInfo.RecDataPageID == RECDATAPAGEID)&& \
//                  (PwrupPageSPAInfo.RecDataLastPageID == 0xffff));
//            
//            SaveDataEndAddr = SaveDataAddr;
//            RePlayDataAddr  = SaveDataEndAddr;
//            SaveRecDataAddrToPageSPA(ReadDataAddr,SaveDataEndAddr);
//            DrvNandFlashAddressIncrement(FLASH_WRITE,&SaveDataAddr);
//            DrvNandFlashWritePageAddress(SaveDataAddr);
//            /* do flash read command juest for reducing power consume, no actual use */
//            ReadCurPageSPARecDataAddr(SaveDataEndAddr,&RecDataAddrInfo);        
//        }
//    }
}

uint8_t ReadRadio_CurrentZone(void)
{
   SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
	
   FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
   uint8_t RadioSetZoneChanelLoad[5];
	
	 uint8_t  CurrentRadioChannel = 0xFF ;
	

	 RadioSetZoneChanelLoad[0] =  0x01 ;
	
	 RadioSetZoneChanelLoad[1] =  0x00;
	
	 RadioSetZoneChanelLoad[2] =  0x00 ;
	
   RadioSetZoneChanelLoad[3] =  0x00 ;
	
	 RadioSetZoneChanelLoad[4] =  0x00 ;
			
	 printf("\r\n Radio ZoneChanel Read Command!\r\n");
				
   while(1)
	 {		 	
					CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, RADIO_CHANGE_ZONE_CMD,RADIO_CHANGE_ZONE_REPLY,RadioSetZoneChanelLoad, sizeof(RadioSetZoneChanelLoad), HrnpReplyMsg);
				
					if(SEND_RESULT_FAILED == CheckResult)
					{
							SystemStatus = STATUS_SYSTEM_FREE ;
						  return CurrentRadioChannel ;
					}
					else if(SEND_RESULT_SUCCESSFUL == CheckResult)
					{
							SystemStatus = STATUS_SYSTEM_FREE ;
						  CurrentRadioChannel = HrnpReplyMsg->RcvFrame[28] ;	
							return CurrentRadioChannel ;
					}        
	}
}
void RadioSelfReset(void);

bool RadioChange_Zone(uint8_t operation,uint16_t zone,uint16_t chanel)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
    uint8_t RadioSetZoneChanelLoad[5];
    bool result = FALSE;

		RadioSetZoneChanelLoad[0] =    operation ;
	
		RadioSetZoneChanelLoad[1] =    zone&0xFF ;
	
		RadioSetZoneChanelLoad[2] =  (zone>>8)&0xFF ;
	
    RadioSetZoneChanelLoad[3] =    chanel&0xFF ;
	
	  RadioSetZoneChanelLoad[4] =  (chanel>>8)&0xFF ;
		
   while(1)
	 {		 
					CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, RADIO_CHANGE_ZONE_CMD,RADIO_CHANGE_ZONE_REPLY,RadioSetZoneChanelLoad, sizeof(RadioSetZoneChanelLoad), HrnpReplyMsg);
				
					if(SEND_RESULT_FAILED == CheckResult)
					{
								SystemStatus = STATUS_SYSTEM_FREE ;
						
						    if(operation==1)
								{
								         Sys_Soft_Reset() ; 
									  //   RadioSelfReset() ; 
								}
								return result ;
					}
					else if(SEND_RESULT_SUCCESSFUL == CheckResult)
					{
								SystemStatus = STATUS_SYSTEM_FREE ;
								result = TRUE ;
						   if(operation==0)
							 {		
								    WorkXinDaoNum = chanel ; 
										printf("\r\n Set ZoneChanel %d Set Command Ok!\r\n",chanel);
							 }	
							 else if(operation==1)
							 {		
								    WorkXinDaoNum = HrnpReplyMsg->RcvFrame[27];
								    printf("\r\n Read ZoneChanel---%d\r\n",WorkXinDaoNum);
							 }
							 return result ;
					}        
    
	}
}

/***************************************************************************
Function name   :Radio_Key_PTT_Congration
Description     :PTTKey Con 
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool Radio_Key_PTT_Congration(uint8_t ConStyle)
{
	
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL ;
	
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
		uint8_t KeyOperationNotifyRq_P[] = {0x01, 0x03, 0x02};
		
    bool    result = FALSE ;
		
		KeyOperationNotifyRq_P[2] = ConStyle ;
		
	  while(1)
		{
						CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, KEY_OPERATION_NOTI, KEY_OPERATION_NOTI_REPLY, KeyOperationNotifyRq_P, sizeof(KeyOperationNotifyRq_P), HrnpReplyMsg);
						
			      if(SEND_RESULT_FAILED == CheckResult)
						{
									 SystemStatus = STATUS_SYSTEM_FREE ;
									 printf("\r\n Key_Opration_Radio_Key_PTT_Congration By Radio--%d Failed !\r\n",ConStyle) ; 
									 return result ;
						}
						else if(SEND_RESULT_SUCCESSFUL == CheckResult)
						{
									SystemStatus = STATUS_SYSTEM_FREE ;
									printf("\r\n Key_Opration_Radio_Key_PTT_Congration By Radio--%d Succeed !\r\n",ConStyle) ;   
									return result ;
						}				
    }
    return result;
}

/***************************************************************************
Function name   :RadioPttCon
Description     :PTT Con 
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool RadioSpkCon(uint8_t Enable)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch1;
    uint8_t SPKConPayLoad[1] = {0x01};
    bool result = FALSE;
		
		SPKConPayLoad[0] = Enable ;

	  while(1)
		{
						CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, SPK_ONLYCON_OB,SPK_ONLYCON_OBREPLY,SPKConPayLoad, sizeof(SPKConPayLoad), HrnpReplyMsg);
					
						if(SEND_RESULT_FAILED == CheckResult)
						{
									SystemStatus = STATUS_SYSTEM_FREE;
									printf("\r\nSend RadioSpkCon_Only Con By OB Command Fail!\r\n") ;
									return result;
						}
						else if(SEND_RESULT_SUCCESSFUL == CheckResult)
						{
									SystemStatus = STATUS_SYSTEM_FREE;
							    if(Enable==1)
									{
									      printf("\r\n RadioSpkCon _Only Control By OB Enable Command Ok!\r\n") ;
									}
									else if(Enable==0)
									{
									      printf("\r\n RadioSpkCon_Only Control By OB Disable Command Ok!\r\n") ;
									}
									result = TRUE;
									return result;
						} 
    }
  
    return result;
}

/***************************************************************************
Function name   :RadioPttCon
Description     :PTT Con 
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool RadioPttCon(uint8_t PttEnable)
{
//    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
//	
//    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
//	
//    uint8_t RadioPttPayLoad[2] = {0x03,0x03};

//    bool result = FALSE;
//		
//		RadioPttPayLoad[1] = PttEnable ;

//	  while(1)
//		{
//						CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, Radio_PTT_Con,Radio_PTT_Con_REPLY,RadioPttPayLoad, sizeof(RadioPttPayLoad), HrnpReplyMsg);
//					
//						if(SEND_RESULT_FAILED == CheckResult)
//						{
//							
//									SystemStatus = STATUS_SYSTEM_FREE ;
//							
//							    if(PttEnable==1)
//									{
//										    printf("\r\n Send Radio PTT Enable Command Fail!\r\n") ;
//									}
//									else
//									{	
//									      printf("\r\n Send Radio PTT Disenable Command Fail!\r\n") ;
//									}
//									
//									return result;
//						}
//						else if(SEND_RESULT_SUCCESSFUL == CheckResult)
//						{
//							
//									SystemStatus = STATUS_SYSTEM_FREE ;
//							
//							    if(PttEnable==1)
//									{
//									      printf("\r\n Send Radio PTT Enable Command Ok!\r\n");
//									}
//									else
//									{
//									      printf("\r\n Send Radio PTT Disenable Command Ok!\r\n");
//									}
//									
//									result = TRUE;
//									return result;
//						} 
//    }
//  
//    return result;
}
/***************************************************************************
Function name   :AudioSpkClose
Description     :close audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool RadioReset(void)
{
//    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
//	
//    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
//	
//    uint8_t *RadioRestePayLoad;
//	
//    bool result = FALSE;

//		printf("Radio Reset Command!!!");	

//	  while(1)
//		{
//						CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, RADIO_RESTART,RADIO_RESTART_REPLY,RadioRestePayLoad, 0, HrnpReplyMsg);
//					
//						if(SEND_RESULT_FAILED == CheckResult)
//						{
//								SystemStatus = STATUS_SYSTEM_FREE;
//							 return result;
//						}
//						else if(SEND_RESULT_SUCCESSFUL == CheckResult)
//						{
//								SystemStatus = STATUS_SYSTEM_FREE;
//								result = TRUE;
//								return result;
//						} 
//    }
//  
//    return result;
}

//7E 04 00 00 20 10 01 0F 00 15 59 96 02 E7 00 02 00 09 01 3F 03
//7E 04 00 00 20 10 01 0F 00 15 59 96 02 E7 00 02 00 09 01 3F 03
//7E 03 00 00 20 10 00 16 00 15 5A 90 02 E7 00 02 00 09 01 3F 03

/***************************************************************************
Function name   :GetRtcTimeData
Description     :Get GetRtcTimeData
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool GetRssiValueCommand(void)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
	
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
    uint8_t GetRssiValueCommand[] = {0x09,0x01};
		
		
    bool result = FALSE;
 	
		printf("\r\n Send GetRssiValueCommand......\r\n") ;
		
    while(1)
		{
								CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, CH_STATUS_CHK_REQ, CH_STATUS_CHK_REQ_REPLY, GetRssiValueCommand,sizeof(GetRssiValueCommand), HrnpReplyMsg);
								if(SEND_RESULT_FAILED == CheckResult)
								{
										 SystemStatus = STATUS_SYSTEM_FREE ;
									   RSSILevel_450M = 0xFF ;
										 printf("\r\nRadio GetRssiValueCommand Fail!\r\n") ;
										 return result;   														
								}
								else if(SEND_RESULT_SUCCESSFUL == CheckResult)
								{
										 SystemStatus = STATUS_SYSTEM_FREE ;
										 
										 RSSILevel_450M = HrnpReplyMsg->RcvFrame[20] ;
  									
									   printf("\r\nRadio GetRssiValueCommand OK---%d!\r\n",RSSILevel_450M) ;
									
										 result = TRUE;
									
										 return result;   
								}
							
	 }
   return result;       
}


/***************************************************************************
Function name   :GetRtcTimeData
Description     :Get GetRtcTimeData
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool GetRtcTimeData(void)
{
	
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
	
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
    uint8_t GetRtcTimeDataValue[1] = {0};
		
		
    bool result = FALSE;
 	
		printf("\r\nSend GetRtcTimeData...\r\n") ;
		
    while(1)
		{
						CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, Radio_RtcTime_Request, Radio_RtcTime_Reply, GetRtcTimeDataValue,sizeof(GetRtcTimeDataValue), HrnpReplyMsg);
			
						if(SEND_RESULT_FAILED==CheckResult)
						{
									 SystemStatus = STATUS_SYSTEM_FREE ;  
									 printf("\r\nRadio GetRtcTimeData Fail!\r\n") ;
									 return result;   
						}
						else if(SEND_RESULT_SUCCESSFUL==CheckResult)
						{
									 SystemStatus = STATUS_SYSTEM_FREE ;
									 printf("\r\nRadio GetRtcTimeData OK!\r\n") ;
									 result = TRUE;
									 RTC_Year = HrnpReplyMsg->RcvFrame[26]<<8|HrnpReplyMsg->RcvFrame[25];
									 RTC_Year = RTC_Year - 2000 ;
									 RTC_Month = HrnpReplyMsg->RcvFrame[24];
									 RTC_Date = HrnpReplyMsg->RcvFrame[23];
									 RTC_Hour = HrnpReplyMsg->RcvFrame[21];
									 RTC_Hour +=8 ;
									 if(RTC_Hour>23)
									 {
												RTC_Hour -= 24 ;
									 }
									 RTC_Minute = HrnpReplyMsg->RcvFrame[20];
									 RTC_Second = HrnpReplyMsg->RcvFrame[19];
									 printf("\r\n%2d-%2d-%2d %2d:%2d:%2d\r\n",RTC_Year,RTC_Month,RTC_Date,RTC_Hour,RTC_Minute,RTC_Second);
									 return result;   
						}		
	 }
   return result;       
}
/***************************************************************************
Function name   :GetPostionData
Description     :Get Postion Data
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool GetPostionData(void)
{
     
}
/****** *********************************************************************
Function name   :OBAudioToRadio_OutRouting_Switch
Description     :OBAudioToRadio_OutRouting_Switch
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool OBMicToRF_OutRouting_Switch(u8 para)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
    uint8_t OBAudioToRadio_OutRouting2[] = {0x01,0x00,0x00};
		
    uint16_t MasterPlayBackCfgItem = OUPUT_PATH_CHANGE2;
		
    bool result = FALSE;
		
		OBAudioToRadio_OutRouting2[2] = para;	
		
    while(1)
		{
							switch(MasterPlayBackCfgItem)
							{      
										case OUPUT_PATH_CHANGE2:
											
													CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, OUPUT_PATH_CHANGE2, OUPUT_PATH_CHANGE2_REPLY, OBAudioToRadio_OutRouting2,sizeof(OBAudioToRadio_OutRouting2), HrnpReplyMsg);
													
										      if(SEND_RESULT_FAILED == CheckResult)
													{
															   SystemStatus = STATUS_SYSTEM_FREE;
														     Sys_Soft_Reset() ; 
														    
													}
													else if(SEND_RESULT_SUCCESSFUL == CheckResult)
													{
															   SystemStatus = STATUS_SYSTEM_FREE;
															   MasterPlayBackCfgItem = SPK_CONTROL_REQUEST;
															   printf("\r\n OBMicToRF_OutRouting_Switch To RF OK!!! \r\n");
														     return result;       
													}
													else
													{
														
													}
										break;
										default:
													break;
							}
	 }
   return result;       
}
/***************************************************************************
Function name   :OBAudioToRadio_InRouting_Switch
Description     :OBAudioToRadio_InRouting_Switch
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool OBAudioToRadio_InRouting_Switch(void)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
	
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
    uint8_t OBAudioToRadio_InRouting[] = {0x01};	
		
    uint16_t MasterPlayBackCfgItem = INPUT_AUDIO_PATH;
		
    bool result = FALSE;
		
    while(1)
		{
							switch(MasterPlayBackCfgItem)
							{
								
										case INPUT_AUDIO_PATH:
											
													CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, INPUT_AUDIO_PATH, INPUT_AUDIO_PATH_REPLY, OBAudioToRadio_InRouting,sizeof(OBAudioToRadio_InRouting), HrnpReplyMsg);
													if(SEND_RESULT_FAILED == CheckResult)
													{
															   SystemStatus = STATUS_SYSTEM_FREE;
														     return result; 
													}
													else if(SEND_RESULT_SUCCESSFUL == CheckResult)
													{
															   SystemStatus = STATUS_SYSTEM_FREE;
														
															   printf("\r\n OBAudioToRadio_InRouting_Switch To RF&SPK OK!!!\r\n");
														
														     return result; 
													}
													break;
													
										default:
													break;
							}
	 }
   return result;       
}
/***************************************************************************
Function name   :OBAudioToRadio_OutRouting_Switch
Description     :OBAudioToRadio_OutRouting_Switch
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool OBAudioToRadio_OutRouting_Switch(void)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
	
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
    uint8_t ChangeAudioOutPutTo_RF[] = {0x02};	
		
    uint16_t MasterPlayBackCfgItem = OUPUT_PATH_CHANGE;
		
    bool result = FALSE;
		
		SpkOpenFlag = 0 ;
		
    while(1)
		{
			        switch(MasterPlayBackCfgItem)
							{      
										case OUPUT_PATH_CHANGE:
											
													CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, OUPUT_PATH_CHANGE, OUPUT_PATH_CHANGE_REPLY, ChangeAudioOutPutTo_RF,sizeof(ChangeAudioOutPutTo_RF), HrnpReplyMsg);
													if(SEND_RESULT_FAILED == CheckResult)
													{
															   SystemStatus = STATUS_SYSTEM_FREE;
													}
													else if(SEND_RESULT_SUCCESSFUL == CheckResult)
													{
															   SystemStatus = STATUS_SYSTEM_FREE;
															   printf("\r\n  OBAudioToRadio_OutRouting_Switch To RF OK!!!\r\n");
														     return result;       
													}
													else
													{
														
													}
										break;
										default:
											
										break;
							}
	 }
   return result;       
}

/***************************************************************************
Function name   :AudioSpkClose
Description     :close audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool AudioSpkUnMute(u8 Enable)
{
    SendResult_t CheckResult              = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
    uint8_t InternalSpkCtrlDisableRq[]    = {0x00,0x02};  //0x00 Enable 0x00 Disable 静音模式
    bool result = FALSE;
		
    while(1)
		{								
							CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, SPK_CONTROL_REQUEST, SPK_CONTROL_REQUEST_REPLY, InternalSpkCtrlDisableRq, sizeof(InternalSpkCtrlDisableRq), HrnpReplyMsg);
							if(SEND_RESULT_FAILED == CheckResult)
							{
									SystemStatus = STATUS_SYSTEM_FREE;
									printf("\r\n Audio SPK UnMute Fail!!!\r\n"); 
							}
							else if(SEND_RESULT_SUCCESSFUL == CheckResult)
							{
									SystemStatus = STATUS_SYSTEM_FREE;
									gSpkDisableReq = 0;
									printf("\r\n Audio SPK UnMute OK!!!\r\n"); 
									return result;														
							}
							else
							{ 
							} 
	 }
    return result;
}
/***************************************************************************
Function name   :AudioSpkConInit
Description     :audio speaker init
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool AudioSpkConInit(void)
{
    SendResult_t CheckResult              = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
    uint8_t InternalSpkCtrlDisableRq[]    = {0x00,0x04};  //0x00 Enable 0x00 Disable 静音模式
    bool result = FALSE;
    while(1)
		{								
							CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, SPK_CONTROL_REQUEST, SPK_CONTROL_REQUEST_REPLY, InternalSpkCtrlDisableRq, sizeof(InternalSpkCtrlDisableRq), HrnpReplyMsg);
							if(SEND_RESULT_FAILED == CheckResult)
							{
									SystemStatus = STATUS_SYSTEM_FREE;
									printf("\r\n Audio SPK Init Fail!!!\r\n"); 
							}
							else if(SEND_RESULT_SUCCESSFUL == CheckResult)
							{
									SystemStatus = STATUS_SYSTEM_FREE;
									gSpkDisableReq = 0;
									printf("\r\n Audio SPK Init OK!!!\r\n"); 
									return result;														
							}
							else
							{
							 
							} 
	   }
		
     return result;
}
/***************************************************************************
Function name   :AudioSpkClose
Description     :close audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool AudioSpkMute(u8 Enable)
{
    SendResult_t CheckResult            = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
    uint8_t InternalSpkCtrlDisableRq[]  = {0x00,0x03};  //0x00 Enable 0x00 Disable 静音模式
    bool result = FALSE ;
		
    while(1)
		{								
						CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, SPK_CONTROL_REQUEST, SPK_CONTROL_REQUEST_REPLY, InternalSpkCtrlDisableRq, sizeof(InternalSpkCtrlDisableRq), HrnpReplyMsg);
						if(SEND_RESULT_FAILED == CheckResult)
						{
								SystemStatus = STATUS_SYSTEM_FREE;
								printf("\r\n Audio SPK Mute Fail!!!\r\n"); 
						}
						else if(SEND_RESULT_SUCCESSFUL == CheckResult)
						{
								SystemStatus = STATUS_SYSTEM_FREE;
								gSpkDisableReq = 0;
								printf("\r\n Audio SPK Mute OK!!!\r\n"); 
								return result;														
						}
	  }
		
    return result;
}
/***************************************************************************
Function name   :AudioSpkClose
Description     :close audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool AudioSpkClose(void)
{
    SendResult_t CheckResult              = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch1;
    uint8_t InternalSpkCtrlDisableRq[]    = {0x00,0x05};  //0x00 Enable 0x01 Disable 静音模式
    uint8_t DisableAudioOutPut[]          = {0x00};
		uint8_t SPKConPayLoad[1] = {0x00};
    uint16_t MasterPlayBackCfgItem = SPK_CONTROL_REQUEST;
    bool result = FALSE;
		
	//	AudioOut_Sel_FSK() ;
			
		if(SpkOpenFlag==0)
		{		 
						return result;
		}
		
    while(1)
		{
						switch(MasterPlayBackCfgItem)
						{
											case SPK_CONTROL_REQUEST:
												
															CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, SPK_CONTROL_REQUEST, SPK_CONTROL_REQUEST_REPLY, InternalSpkCtrlDisableRq, sizeof(InternalSpkCtrlDisableRq), HrnpReplyMsg);
															if(SEND_RESULT_FAILED == CheckResult)
															{
																	SystemStatus = STATUS_SYSTEM_FREE;
																	printf("\r\n Audio SPK Close Fail!!!\r\n");
																  SpkOpenFlag = 1; 
																	return result;														
															}
															else if(SEND_RESULT_SUCCESSFUL == CheckResult)
															{
																	SystemStatus = STATUS_SYSTEM_FREE;
																	MasterPlayBackCfgItem = OUPUT_PATH_CHANGE;
																	gSpkDisableReq = 0;
																  SpkOpenFlag = 0;
																	printf("\r\n Audio SPK Close OK!!!\r\n"); 													
															}
															else
															{
															 
															} 
											break; 		
											case OUPUT_PATH_CHANGE: 
															CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, OUPUT_PATH_CHANGE, OUPUT_PATH_CHANGE_REPLY, DisableAudioOutPut, sizeof(DisableAudioOutPut), HrnpReplyMsg);
															if(SEND_RESULT_FAILED == CheckResult)
															{
																	SystemStatus = STATUS_SYSTEM_FREE;
																	return result;
															}
															else if(SEND_RESULT_SUCCESSFUL == CheckResult)
															{
																	SystemStatus = STATUS_SYSTEM_FREE;
																	MasterPlayBackCfgItem = SPK_ONLYCON_OB;
																	gSpkDisableReq = 0;
																	printf("\r\n OUPUT_PATH_CHANGE Successed!!!\r\n");
																	result = TRUE;
																	SpkOpenFlag = 0  ;		
																	SpkOpenCount = 0 ;
																	MasterPlayBackCfgItem = SPK_ONLYCON_OB;														
															}
															else
															{
																
															}
											break; 
											case SPK_ONLYCON_OB:	
											{												
														CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, SPK_ONLYCON_OB,SPK_ONLYCON_OBREPLY,SPKConPayLoad, sizeof(SPKConPayLoad), HrnpReplyMsg);
														if(SEND_RESULT_FAILED == CheckResult)
														{
																	SystemStatus = STATUS_SYSTEM_FREE;
																	printf("\r\nSend RadioSpkCon_Only Con By OB Command Fail!\r\n") ;
																	return result;
														}
														else if(SEND_RESULT_SUCCESSFUL == CheckResult)
														{
																	SystemStatus = STATUS_SYSTEM_FREE;
																	if(SPKConPayLoad[0]==1)
																	{
																				printf("\r\n RadioSpkCon _Only Control By OB Enable Command Ok!\r\n") ;
																	}
																	else if(SPKConPayLoad[0]==0)
																	{
																				printf("\r\n RadioSpkCon_Only Control By OB Disable Command Ok!\r\n") ;
																	}
																	AudioSpkMute(0)  ;
																	result = TRUE;
																	return result;
														} 
										  }
											default: break;   
							}
	 }
    return result;
}
/***************************************************************************
Function name   :AudioSpkOpen
Description     :open audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool AudioSpkOpen(void)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
    uint8_t ChangeAudioOutPutToSPK[]   = {0x01} ;	      //{0x03};
		uint8_t InternalSpkCtrlEnableRq[]  = {0x00,0x04} ;  //0x02  0x00
    uint16_t MasterPlayBackCfgItem = OUPUT_PATH_CHANGE ;
		uint8_t    i = 0 ;
    bool result = FALSE;	
		
		AudioOut_Sel_SPK();
		
	  if(SpkOpenFlag==1)
	  {
			  return result;
	  }
		
	  RadioSpkCon(1);         // 打开OB独占SPK功能	
		
    while(1)
		{
							switch(MasterPlayBackCfgItem)
							{      
										case OUPUT_PATH_CHANGE:
													CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, OUPUT_PATH_CHANGE, OUPUT_PATH_CHANGE_REPLY, ChangeAudioOutPutToSPK,sizeof(ChangeAudioOutPutToSPK), HrnpReplyMsg);
													if(SEND_RESULT_FAILED == CheckResult)
													{
																	 SystemStatus = STATUS_SYSTEM_FREE;
														       MasterPlayBackCfgItem = SPK_CONTROL_REQUEST;
																	 printf("\r\nAudio Path Change To OB Failed!!!\r\n");
														      
													}
													else if(SEND_RESULT_SUCCESSFUL == CheckResult)
													{
																	 SystemStatus = STATUS_SYSTEM_FREE;
																	 MasterPlayBackCfgItem = SPK_CONTROL_REQUEST;
																	 printf("\r\nAudio Path Change To OB OK!!!\r\n");
														  
													}
													else
													{
													}
										break;
										case SPK_CONTROL_REQUEST:
													CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, SPK_CONTROL_REQUEST, SPK_CONTROL_REQUEST_REPLY, InternalSpkCtrlEnableRq, sizeof(InternalSpkCtrlEnableRq), HrnpReplyMsg);
													if(SEND_RESULT_FAILED == CheckResult)
													{
																	SystemStatus = STATUS_SYSTEM_FREE;
																	printf("\r\n Audio Speaker Open Failed ! \r\n"); 
													}
													else if(SEND_RESULT_SUCCESSFUL == CheckResult)
													{															
																	SystemStatus = STATUS_SYSTEM_FREE;
																	MasterPlayBackCfgItem = OUPUT_PATH_CHANGE;
																	gSpkEnableReq = 0;
																	result = TRUE ;
																	SpkOpenFlag = 1 ;
																	if(SpkOpenCount==0)
																				SpkOpenCount = 10;
																	printf("\r\n Audio Speaker OpenKeepTime %d Success.\r\n",SpkOpenCount) ;	
																	result = HrnpSlaveRcvAndOperateCmdFromMaster() ;                    //接收处理函数
																	for(i=0;i<100;i++)
																	{
																	      delay_ms(1) ;
																		 	  result = HrnpSlaveRcvAndOperateCmdFromMaster() ;              //接收处理函数
																	}
																	return  result ;	
													}
													else
													{
															   
													} 
										break;
										default:
													break;
							}
	 }
   return result;      
}

/***************************************************************************
Function name   :MenuWindowsDis
Description     :open audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool MenuWindowsClose(uint32_t value)
{
			SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL ;
			FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
			bool     result = FALSE ;		
			uint8_t  index = 0 ; 
			uint8_t  DisArry[4] ;
	
			//WindowID 窗口索引号
			DisArry[index++] =  WindowID&0xFF ;  
			DisArry[index++] = (WindowID>>8)&0xFF ;
			DisArry[index++] = (WindowID>>16)&0xFF ;
			DisArry[index++] = (WindowID>>24)&0xFF ;

			while(1)
			{
							CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, Window_Close_Menu, Window_Close_Menu_REPLY, DisArry, sizeof(DisArry), HrnpReplyMsg) ;
				
							if(SEND_RESULT_FAILED == CheckResult)
							{
									SystemStatus = STATUS_SYSTEM_FREE;
									printf("\r\nMenuWindowsClose Index--%d Fail!\r\n",WindowID); 
								  if(WindowID==2)
									{
									    WindowID = 1 ;
									
									}
									else
									{
									     WindowID = 2 ;	 
									}
									
									DisArry[index++] =  WindowID&0xFF ;  
									DisArry[index++] = (WindowID>>8)&0xFF ;
									DisArry[index++] = (WindowID>>16)&0xFF ;
									DisArry[index++] = (WindowID>>24)&0xFF ;	
									
									CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, Window_Close_Menu, Window_Close_Menu_REPLY, DisArry, sizeof(DisArry), HrnpReplyMsg) ;
									
									if(SEND_RESULT_FAILED == CheckResult)
									{
												SystemStatus = STATUS_SYSTEM_FREE;
												printf("\r\nMenuWindowsClose Index--%d Fail Again!\r\n",WindowID);
									}
									
									return result ;		
							}
							else if(SEND_RESULT_SUCCESSFUL == CheckResult)
							{
									SystemStatus = STATUS_SYSTEM_FREE;
									printf("\r\nMenuWindowsClose Index--%d Ok\r\n",WindowID);
									result = TRUE ;	
									return result;						
							}		
			}   
}
void MenuWindows_ParaAckMenu(u8 command,u16 FengYaValue,u32 ReceTime)
{			
	        u8  Opration = 0 ;
					u16  year;
					u8   mon;
					u8   mday;
					u8   hour;
					u8   min;
					u8   sec ;
	        u8   TempIp[4]= {0};
					
					
					TempIp[0] = (ReceTime>>24)&0xFF ;
					TempIp[1] = (ReceTime>>16)&0xFF ;
					TempIp[2] = (ReceTime>>8)&0xFF ;
					TempIp[3] = (ReceTime)&0xFF ;
	
	        MenuWindows_Refresh();
	
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

				  if(command==0xD9)
					{
						  HighLightItem = 2 ;
					    sprintf(DisTemp,"%s","参数查询");	
						
							GetRtcTimeData();	
              ReceTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second ;    /*获取时间*/					

					}
					else if(command==0xDA)
          {
					    sprintf(DisTemp,"%s","参数设置");	
						  HighLightItem = 2 ;
					}
					else if(command==0xDB)
          {
					    sprintf(DisTemp,"%s","参数应答");
 						  HighLightItem = 3 ;
					}
					
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
				
					Item1_ItemLen = 0 ;
				
					sprintf(DisTemp,"%s","列尾");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
					sprintf(DisTemp,": %06X",IDNUM);		
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					Item2_ItemLen = 0 ;
				
					sprintf(DisTemp,"%s","操作");
					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
					sprintf(DisTemp,": ");		
          Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
							
					Opration = 	FengYaValue&0xFF ;

					Item2_ItemLen = 3 ;
					if(command==0xD9)
					{
								sprintf(DisTemp,"%s","参数查询");
					      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
					}
					else if(command==0xDA)
					{
								sprintf(DisTemp,"%s","参数设置");
					      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
					}
					else if(command==0xDB)
					{
								sprintf(DisTemp,"%s","参数应答");
					      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
					}
					
					if(Opration==0)
					{										
						
												year =  (ReceTime>>26)&0x3F;
												mon =   (ReceTime>>22)&0x0F; 
												mday =  (ReceTime>>17)&0x1F;
												hour =  (ReceTime>>12)&0x1F;
												min =    (ReceTime>>6)&0x3F;
												sec =    ReceTime&0x3F;
										
												Item3_ItemLen =0 ;
												sprintf(DisTemp,"%s","日期");
												Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
											
												sprintf(DisTemp,": ");		
												Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	

												Item4_ItemLen = 0 ;
												sprintf(DisTemp,"%s","时间");
												Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
												sprintf(DisTemp,": ");		
												Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
												
												if(command==0xD9)
												{
															Item3_ItemLen =  3  ;
															sprintf(DisTemp,"%s ","**:**:**");
															Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
															
															Item4_ItemLen =  3  ;
													
															sprintf(DisTemp,"%s ","--:--:--");
															Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;																	
												}
												else
												{
															Item3_ItemLen =  3  ;
															sprintf(DisTemp,"%02u:%02u:%02u  ",year,mon,mday);
															Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
															
															Item4_ItemLen =  3  ;
														
															sprintf(DisTemp,"%02u:%02u:%02u  ",hour,min,sec);
															Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
												}												
					}
          else if(Opration==1)
					{
										Item3_ItemLen =0 ;
										sprintf(DisTemp,"%s","序号");
										Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
									
										sprintf(DisTemp,": ");		
										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
								

						       	if(command==0xD9)
										{
										     sprintf(DisTemp,"**");
										}
										else
										{
										     sprintf(DisTemp,"%02d",TempIp[0]);
										}	
						
										
										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;															
					}
					
				  else if(Opration==2)
					{

//										Item3_ItemLen =0 ;
//										sprintf(DisTemp,"%s","地址");
//										Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
//									
//										sprintf(DisTemp,": ");		
//										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
//								
//										sprintf(DisTemp,"%02d",ReceTime&0xFF);
//										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;						
					
					}
				  else if(Opration==3)
					{
//										sprintf(DisTemp,"%s","查询备");
//										Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
//						
//										sprintf(DisTemp,"%s","DNS");	
//										Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;		
					}
					else if(Opration==4)
					{
//										sprintf(DisTemp,"%s","查询");
//										Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	

//										sprintf(DisTemp,"%s","RMS IP");	
//										Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
					}
	        else if(Opration==5)
					{
//										sprintf(DisTemp,"%s","公网端口");
//					          Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
//						
						        Item3_ItemLen = 0 ;
										sprintf(DisTemp,"%s","端口");
					          Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
						
										sprintf(DisTemp,":");		
									  Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;

						        sprintf(DisTemp,"%04d",(ReceTime>>16)&0xFFF);		
								    Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
						
					}
	        else if(Opration==6)  
					{
//								 sprintf(DisTemp,"%s","公网");
//								 Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
						
										sprintf(DisTemp,"%s","IP");	
										Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;

					}
					printf("Receivee Opration Value---%d", Opration);
					if(Opration==2||Opration==3||Opration==4||Opration==6)
					{				
									 Item3_ItemLen = 0 ;
									 sprintf(DisTemp,"%s","地址") ;
									 Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
									 sprintf(DisTemp,":");		
									 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
						
						       if(command==0xD9)
									 {
									 			 Item3_ItemLen = 3 ;
												 sprintf(DisTemp,"         ");		
												 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
									 }
									 else
									 {
												 Item3_ItemLen = 3 ;
									
												 sprintf(DisTemp,"%d.%d.%d.%d",TempIp[0],TempIp[1],TempIp[2],TempIp[3]);		
												 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;										 
									 }			
					}
					else if(command==0xDA||command==0xDB)
					{
					       HighLightItem = 3 ;
					}
					
					Button1Value =  0x0000 ;
					Button2Value =  0x0001 ;
					
					MenuWindowsDis(WindowID);	
}
void MenuWindows_ParaReadMenu2(void)
{
					MenuWindows_Refresh();
				
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;  

					sprintf(DisTemp,"%s","参数查询");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","2====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			

					Item1_ItemLen = 0 ;
	
					sprintf(DisTemp,"%s","7.");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
				
					sprintf(DisTemp,"%s","查询公网");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					sprintf(DisTemp,"%s","IP");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
					
					Button1Value =  0x0000 ;
					Button2Value =  0x0001 ;
					
			
					MenuWindowsDis(WindowID);	
}

void MenuWindows_ParaReadMenu1(void)
{
				//	MenuWindows_Clear();
				
	        MenuWindows_Refresh();
	
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

				
					sprintf(DisTemp,"%s","参数查询");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","1====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			

					Item1_ItemLen = 0 ;
	
					sprintf(DisTemp,"%s","1.");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
				
					sprintf(DisTemp,"%s","查询时间");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					Item2_ItemLen = 0 ;
					sprintf(DisTemp,"%s","2.");	
					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
					sprintf(DisTemp,"%s","查询");
					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;		
          sprintf(DisTemp,"%s","APN");	
					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;					
					
					Item3_ItemLen = 0 ;
					sprintf(DisTemp,"%s","3.");	
					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
					sprintf(DisTemp,"%s","查询主用");
					Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;				
          sprintf(DisTemp,"%s","DNS");	
					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;					

					Item4_ItemLen = 0 ;
					sprintf(DisTemp,"%s","4.");	
					Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
					sprintf(DisTemp,"%s","查询备用");
					Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;					
          sprintf(DisTemp,"%s","DNS");	
					Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;		

					Item5_ItemLen = 0 ;
					sprintf(DisTemp,"%s","5.");	
					Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;
					sprintf(DisTemp,"%s","查询");
					Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	

					sprintf(DisTemp,"%s","RMSIP");	
					Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","地址");
					Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
					
					Item6_ItemLen = 0 ;
					sprintf(DisTemp,"%s","6.");	
					Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","查询公网端口");
					Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	


					Button1Value =  0x0002 ;
					Button2Value =  0x0001 ;
						
					MenuWindowsDis(WindowID);	
}
void MenuWindows_ParaRead(void)
{
	      u8    key = 0 ;
	      u8   CurrentSelIndex = 1 ;
	
	      u8    FanHuiKeyCount = 1 ;
	
		    HighLightItem	= 1 ;
	
        MenuWindows_ParaReadMenu1();
				
				KeyActiveStru.KeyType = 0xFF ;
				
				KeyActiveStru.KeyOperate = 0xFF ;
				
	      while(1)
				{		
					
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
						if(Dmr400M_DataReceFlag==1)    
						{		     
									 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
									 Dmr400MReceFrame[3]  = 0xFF ;
									 Dmr400MReceFrame[9]  = 0xFF ;
									 Dmr400M_DataReceFlag = 0x00 ;
						}

		        HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
			    	if(KeyActiveStru.KeyOperate!=0xFF)	
						{							
							     if(KeyActiveStru.KeyOperate==0x02)
								   {											 
													 key = CovertKeyValue(KeyActiveStru.KeyType) ;
											      
										       AudioSpkOpen();	
										 
										       printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;

										      if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)               //返回
													{
														    wtn_play_voice_one(50);
														
														    if(FanHuiKeyCount>0) 
																{
																     FanHuiKeyCount--;
																	   MenuWindows_ParaReadMenu1();
																}
														    else
																{
																     return ;
																}
														     
																 
													}
												  else if(key>=KEY_SHUZI1_VALUE&&key<=KEY_SHUZI7_VALUE)
												  {
														       wtn_play_voice(key);
                                   CurrentSelIndex = key ;
																	 SendCommandOprationIndex = CurrentSelIndex - 1  ;
														       MenuWindows_ParaAckMenu(0xD9, SendCommandOprationIndex,ReceTime)  ;
														       FanHuiKeyCount = 1 ; 
																	// Dmr400M_DataSendDataFun(0xD9) ;
												  }
												  else if(key==KEY_QUEREN_VALUE||key==KEY_RIGHT_VALUE)               //返回
												  {  
														       wtn_play_voice_one(28);
																	 SendCommandOprationIndex = CurrentSelIndex - 1  ;
														       MenuWindows_ParaAckMenu(0xD9, SendCommandOprationIndex,ReceTime)  ;
														       FanHuiKeyCount = 1 ;  
												  }
                          else if(key==KEY_CHAXUN_VALUE)               //返回
													{
														      wtn_play_voice_one(33);
																	SendCommandOprationIndex = CurrentSelIndex - 1  ;
														      MenuWindows_ParaAckMenu(0xD9, SendCommandOprationIndex,ReceTime)  ;
																	Dmr400M_DataSendDataFun(0xD9) ;
														
													}														
												
												if(key==KEY_UP_VALUE)  
												{
													  wtn_play_voice_one(28);
													
														if(CurrentSelIndex>1)
																 CurrentSelIndex-- ;
														else
																 CurrentSelIndex = 7 ;
														
														 if(CurrentSelIndex==7)
														 {
														      HighLightItem = 1 ;
															    MenuWindows_ParaReadMenu2();	
														 }
														 else
														 {
														      HighLightItem = CurrentSelIndex ;
															 
															    MenuWindows_ParaReadMenu1();				
														 }
												}
												
												if(key==KEY_DOWN_VALUE)  
												{
													  wtn_play_voice_one(28);
														if(CurrentSelIndex<7)
																 CurrentSelIndex++ ;
														else
																 CurrentSelIndex =1 ;
																	
														 if(CurrentSelIndex==7)
														 {
														      HighLightItem = 1 ;
															    MenuWindows_ParaReadMenu2();	
														 }
														 else
														 {
														      HighLightItem = CurrentSelIndex ;
															    MenuWindows_ParaReadMenu1();		
														 }		 
												}
												else if(key==KEY_XING_VALUE||key==KEY_JING_VALUE)
												{
                                 wtn_play_voice_one(28);
												}	
											   KeyActiveStru.KeyType = 0xFF ;											 
						        }
									 
									   key = 0xFF ; 	 
									   KeyActiveStru.KeyOperate = 0xFF ; 	 
            } 
						
						if(T1S_Flag==1)
						{  
									RUN_LED__TURN ;
									IWDG_ReloadCounter();    //喂狗
						}						 					
			 }
}

void MenuWindows_ParaSendMenu(u8 command,u16 FengYaValue,u32 ReceTime)
{
	        u8  Opration = 0 ;
					u16  year;
					u8   mon;
					u8   mday;
					u8   hour;
					u8   min;
					u8   sec ;
	        u8   TempIp[4]= {0};
					

	        MenuWindows_Refresh();
	
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

				  if(command==0xD9)
					{
					     sprintf(DisTemp,"%s","参数查询");	
					}
				  if(command==0xDA)
					{
					     sprintf(DisTemp,"%s","参数设置");	
					}
					if(command==0xDB)
					{
					     sprintf(DisTemp,"%s","参数应答");	
					}
					
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
				
					Item1_ItemLen = 0 ;
				
					
					sprintf(DisTemp,"%s","列尾");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
					sprintf(DisTemp,": %06X",IDNUM);		
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					Item2_ItemLen = 0 ;
				
					sprintf(DisTemp,"%s","操作");
					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
					sprintf(DisTemp,": ");		
          Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
						
						
					Opration = 	FengYaValue&0xFF ;
					
					if(Opration==0)
					{
					    	sprintf(DisTemp,"%s","查询时间");
					      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
						
								year =  (ReceTime>>26)&0x3F;
								mon =   (ReceTime>>22)&0x0F; 
								mday =  (ReceTime>>17)&0x1F;
								hour = (ReceTime>>12)&0x1F;
								min=  (ReceTime>>6)&0x3F;
								sec=  ReceTime&0x3F;
						
						    Item3_ItemLen =0 ;
								sprintf(DisTemp,"%s","日期");
								Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
							
								sprintf(DisTemp,": ");		
								Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
						
								sprintf(DisTemp,"%02u:%02u:%02u",year,mon,mday);
								Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
						

						    Item4_ItemLen =0 ;
								sprintf(DisTemp,"%s","时间");
								Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
							
								sprintf(DisTemp,": ");		
								Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
								
								sprintf(DisTemp,"%02u:%02u:%02u",hour,min,sec);
								Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;

					}
          else if(Opration==1)
					{
								sprintf(DisTemp,"%s","查询");
								Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;		
								sprintf(DisTemp,"%s","APN");	
								Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
						
								Item3_ItemLen =0 ;
								sprintf(DisTemp,"%s","序号");
								Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
							
								sprintf(DisTemp,": ");		
								Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
						
								sprintf(DisTemp,"%02d",ReceTime&0xFF);
								Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
					}
					
				  else if(Opration==2)
					{
										sprintf(DisTemp,"%s","查询主用");
										Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;				
										sprintf(DisTemp,"%s","DNS");	
										Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;

										Item3_ItemLen =0 ;
										sprintf(DisTemp,"%s","地址");
										Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
									
										sprintf(DisTemp,": ");		
										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
								
										sprintf(DisTemp,"%02d",ReceTime&0xFF);
										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;						
					
					}
				  else if(Opration==3)
					{
										sprintf(DisTemp,"%s","查询备用");
										Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
						
										sprintf(DisTemp,"%s","DNS");	
										Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;		

					}
					else if(Opration==4)
					{
										sprintf(DisTemp,"%s","查询");
										Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	

										sprintf(DisTemp,"%s","RMS IP");	
										Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
					}
	        else if(Opration==5)
					{
										sprintf(DisTemp,"%s","查询公网端口");
					          Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
						
										sprintf(DisTemp,"%s","端口");
					          Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
						
										sprintf(DisTemp,": ");		
									  Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;

										TempIp[0] = ReceTime>>24&0xFF ;
										TempIp[1] = ReceTime>>16&0xFF ;
						
						        sprintf(DisTemp,"%d.%d",TempIp[0],TempIp[1]);		
								    Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
						
					}
	        else if(Opration==6)
					{
										sprintf(DisTemp,"%s","查询公网");
										Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
						
										sprintf(DisTemp,"%s","IP");	
						
										Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
					}
					
					if(Opration==2||Opration==3||Opration==4||Opration==6)
					{				
									 Item3_ItemLen = 0 ;
									 sprintf(DisTemp,"%s","地址") ;
									 Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
									 sprintf(DisTemp,": ");		
									 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
						
									 TempIp[0] = ReceTime>>24&0xFF ;
									 TempIp[1] = ReceTime>>16&0xFF ;
									 TempIp[2] = ReceTime>>8&0xFF ;
									 TempIp[3] = ReceTime&0xFF ;
						
									 sprintf(DisTemp,"%d.%d.%d.%d",TempIp[0],TempIp[1],TempIp[2],TempIp[3]);		
									 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
					}

					Button1Value =  0x0000 ;
					Button2Value =  0x0001 ;
					
					MenuWindowsDis(WindowID);	
}

void MenuWindows_ParaSetMenu2(void)
{
					MenuWindows_Refresh();
				
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

					sprintf(DisTemp,"%s","参数设置");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			

	
					Item1_ItemLen = 0 ;
	
					sprintf(DisTemp,"%s","7.");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
				
					sprintf(DisTemp,"%s","设置公网");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					sprintf(DisTemp,"%s","IP");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
						
					Button1Value =  0x0000 ;
					Button2Value =  0x0001 ;
					
			
					MenuWindowsDis(WindowID);	
}
void MenuWindows_DeviceSetMenu2(void)
{
					MenuWindows_Refresh();
				
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

				
					sprintf(DisTemp,"%s","设备控制");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","2====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			
					Item1_ItemLen = 0 ;
	

					Item1_ItemLen = 0 ;
					sprintf(DisTemp,"%s","7.");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","配置");	
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","频组");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;

					sprintf(DisTemp,"%s","2 ");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
					

//					Item2_ItemLen = 0 ;
//					sprintf(DisTemp,"%s","8.");	
//					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
//					sprintf(DisTemp,"%s","查询");
//					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;		
//          sprintf(DisTemp,"%s","GSMR");	
//					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;					
//					sprintf(DisTemp,"%s","状态");
//					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
	
					Button1Value =  0x0002 ;
					Button2Value =  0x0001 ;
					
					MenuWindowsDis(WindowID);	
}
void MenuWindows_DeviceSetMenu1(void)
{
					MenuWindows_Refresh();
				
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

				
					sprintf(DisTemp,"%s","主机配置");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","1====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			
					Item1_ItemLen = 0 ;
	
					sprintf(DisTemp,"%s","1.");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
				
					sprintf(DisTemp,"%s","开启常亮");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					Item2_ItemLen = 0 ;
					sprintf(DisTemp,"%s","2.");	
					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
					sprintf(DisTemp,"%s","关闭常亮");
					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;		


					Item3_ItemLen = 0 ;
					sprintf(DisTemp,"%s","3.");	
					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
					sprintf(DisTemp,"%s","开启长发");
					Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	

					Item4_ItemLen = 0 ;
					sprintf(DisTemp,"%s","4.");	
					Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
					sprintf(DisTemp,"%s","开启定位");
					Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	

					Item5_ItemLen = 0 ;
					sprintf(DisTemp,"%s","5.");	
					Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;
					sprintf(DisTemp,"%s","关闭定位");
					Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	

					Item6_ItemLen = 0 ;
					sprintf(DisTemp,"%s","6.");	
					Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","配置");	
					Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","频组");
					Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;

					sprintf(DisTemp,"%s","1 ");	
					Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;
					
					Button1Value =  0x0002 ;
					Button2Value =  0x0001 ;
					
					MenuWindowsDis(WindowID);	
}

void MenuWindows_DeviceSet(void)
{
	      u8    key = 0 ;
	      u8    CurrentSelIndex = 1;
	      u8    FanHuiKeyCount = 0 ;
	
	      HighLightItem	= 0 ;
        MenuWindows_DeviceSetMenu1();
	
        Item2_ItemLen = 4 ;
				
				KeyActiveStru.KeyType = 0xFF ;
				KeyActiveStru.KeyOperate = 0xFF ;
				
	      while(1)
				{
					
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
						if(Dmr400M_DataReceFlag==1)    
						{		     
							     
										 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
										 Dmr400MReceFrame[3]  = 0xFF ;
										 Dmr400MReceFrame[9]  = 0xFF ;
										 Dmr400M_DataReceFlag = 0x00 ;
							
							       FanHuiKeyCount =  1 ;
						}
						
		        HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
			    	if(KeyActiveStru.KeyOperate!=0xFF)	
						{							
							     if(KeyActiveStru.KeyOperate==0x02)
								   {			
										 
													key = CovertKeyValue(KeyActiveStru.KeyType) ;
											      
										      AudioSpkOpen();	
										 
										      printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;

										 			if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)               //返回
													{
														     wtn_play_voice_one(50);
														     if(FanHuiKeyCount>0)
																 {
														          FanHuiKeyCount-- ;
																	    MenuWindows_DeviceSetMenu1();			
																 }	
                                 else
																 {																	 
																	     return ;	 
																 } 
													}
										 			else if(key>=KEY_SHUZI1_VALUE&&key<=KEY_SHUZI8_VALUE)
												  {

														       if(key == KEY_SHUZI1_VALUE)
																	 {
																		          SendFengYaValue = 0x0001 ;
																	     			  Dmr400M_DataSendDataFun(0x01)	;											 
																	 }
																	 else if(key==KEY_SHUZI2_VALUE)
																	 {
	                                      			SendFengYaValue = 0x0000 ;
																	     			  Dmr400M_DataSendDataFun(0x01)	;			
																	 }
																	 else if(key==KEY_SHUZI3_VALUE)
																	 {
	                                      	    SendFengYaValue = 0x0164 ;
																	     			  Dmr400M_DataSendDataFun(0x01)	;		
																	 }
																	 else if(key == KEY_SHUZI4_VALUE)
																	 {
																	     				SendFengYaValue = 0x0201;
																	     			  Dmr400M_DataSendDataFun(0x01)	;												 
																	 }
																	 else if(key==KEY_SHUZI5_VALUE)
																	 {
	                                      			SendFengYaValue = 0x0200;
																	     			  Dmr400M_DataSendDataFun(0x01)	;	
																	 }
																	 else if(key==KEY_SHUZI6_VALUE)
																	 {
	                                     	     SendFengYaValue = 0x0301;
																	     			 Dmr400M_DataSendDataFun(0x01)	;	 
																	 }
																	 else if(key==KEY_SHUZI7_VALUE)
																	 {
	                                      	   SendFengYaValue = 0x0302;
																	     			 Dmr400M_DataSendDataFun(0x01)	;	 
																	 }
																	  else if(key==KEY_SHUZI8_VALUE)
																	 {
	                                      	    SendFengYaValue = 0x01FF;
																	     			  Dmr400M_DataSendDataFun(0x02)	;	 
																	 }
                                   else
																	 {
																		           wtn_play_voice_one(28);  			 
																	 }
																	 
																	 MenuWindows_DeviceSetMenu1();					 
																

												  }
													else if(key==KEY_QUEREN_VALUE||key==KEY_RIGHT_VALUE)               //返回
													{
															     if(CurrentSelIndex == KEY_SHUZI1_VALUE)
																	 {
																		          SendFengYaValue = 0x0001 ;
																	     			  Dmr400M_DataSendDataFun(0x01)	;											 
																	 }
																	 else if(CurrentSelIndex==KEY_SHUZI2_VALUE)
																	 {
	                                      			SendFengYaValue = 0x0000 ;
																	     			  Dmr400M_DataSendDataFun(0x01)	;			
																	 }
																	 else if(CurrentSelIndex==KEY_SHUZI3_VALUE)
																	 {
	                                      	    SendFengYaValue = 0x0164 ;
																	     			  Dmr400M_DataSendDataFun(0x01)	;		
																	 }
																	 else if(CurrentSelIndex == KEY_SHUZI4_VALUE)
																	 {
																	     				SendFengYaValue = 0x0201;
																	     			  Dmr400M_DataSendDataFun(0x01)	;												 
																	 }
																	 else if(CurrentSelIndex==KEY_SHUZI5_VALUE)
																	 {
	                                      			SendFengYaValue = 0x0200;
																	     			  Dmr400M_DataSendDataFun(0x01)	;	
																	 }
																	 else if(CurrentSelIndex==KEY_SHUZI6_VALUE)
																	 {
	                                     	     SendFengYaValue = 0x0301;
																	     			 Dmr400M_DataSendDataFun(0x01)	;	 
																	 }
																	 else if(CurrentSelIndex==KEY_SHUZI7_VALUE)
																	 {
	                                      	   SendFengYaValue = 0x0302;
																	     			 Dmr400M_DataSendDataFun(0x01)	;	 
																	 }
																	  else if(CurrentSelIndex==KEY_SHUZI8_VALUE)
																	 {
	                                      	    SendFengYaValue = 0x01FF;
																	     			  Dmr400M_DataSendDataFun(0x02)	;	 
																	 }
                                   else
																	 {
																		   wtn_play_voice_one(28);  			 
																	 }
																	 
																	 MenuWindows_DeviceSetMenu1();
												}	    
												 else if(key==KEY_CHAXUN_VALUE)               //发送命令
												 {	    											      
                                    wtn_play_voice_one(28);
												 }
												 else if(key==KEY_JING_VALUE)
												 {
                                   wtn_play_voice_one(28);
												 }
												 else if(key==KEY_UP_VALUE)  
												 {	
																wtn_play_voice_one(28);
												 
																if(CurrentSelIndex>1)
																		 CurrentSelIndex-- ;
																else
																		 CurrentSelIndex = 7 ;
																
																 if(CurrentSelIndex==7)
																 {
																			HighLightItem = 1 ;
																			MenuWindows_DeviceSetMenu2();	
																 }
																 else
																 {
																			HighLightItem = CurrentSelIndex ;	 
																			MenuWindows_DeviceSetMenu1();				
																 }
													}
												
												if(key==KEY_DOWN_VALUE)  
												{
													
																wtn_play_voice_one(28);
														
																if(CurrentSelIndex<7)
																{
																		 CurrentSelIndex++ ;
																	   if(CurrentSelIndex==7)
																		 {
																						HighLightItem = 1 ;
																						MenuWindows_DeviceSetMenu2();	
																		 }
																		 else
																		 {
																						HighLightItem = CurrentSelIndex ;
																						MenuWindows_DeviceSetMenu1();	
																		 }
																}
																else
																{
																						HighLightItem = 1 ;
																	          CurrentSelIndex  = 1 ;
																						MenuWindows_DeviceSetMenu1();	
																}					
												}	
												
												else if(key==KEY_XING_VALUE)
												{
																wtn_play_voice_one(28);
												}	


													KeyActiveStru.KeyType = 0xFF ;											 
						        }
									 
									   KeyActiveStru.KeyOperate = 0xFF ; 	 
            } 
						
						if(T1S_Flag==1)
						{  
									RUN_LED__TURN ;
							
									IWDG_ReloadCounter();    //喂狗
							
						}						 					
			 }
}

void MenuWindows_ParaSetMenu1(void)
{
					MenuWindows_Refresh();
				
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

				
					sprintf(DisTemp,"%s","参数设置");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","1====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			
					Item1_ItemLen = 0 ;
	
					sprintf(DisTemp,"%s","1.");	
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
				
					sprintf(DisTemp,"%s","设置时间");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					Item2_ItemLen = 0 ;
					sprintf(DisTemp,"%s","2.");	
					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
					sprintf(DisTemp,"%s","设置");
					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;		
          sprintf(DisTemp,"%s","APN");	
					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;					
					
					Item3_ItemLen = 0 ;
					sprintf(DisTemp,"%s","3.");	
					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
					sprintf(DisTemp,"%s","设置主用");
					Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;				
          sprintf(DisTemp,"%s","DNS");	
					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;					

					Item4_ItemLen = 0 ;
					sprintf(DisTemp,"%s","4.");	
					Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
					sprintf(DisTemp,"%s","设置备用");
					Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;					
          sprintf(DisTemp,"%s","DNS");	
					Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;		

					Item5_ItemLen = 0 ;
					sprintf(DisTemp,"%s","5.");	
					Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;
					sprintf(DisTemp,"%s","设置");
					Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	

					sprintf(DisTemp,"%s","RMS IP");	
					Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","地址");
					Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
					
					Item6_ItemLen = 0 ;
					sprintf(DisTemp,"%s","6.");	
					Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;
					
					sprintf(DisTemp,"%s","设置公网端口");
					Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	

						
					Button1Value =  0x0002 ;
					Button2Value =  0x0001 ;
					
					MenuWindowsDis(WindowID);	
}
void MenuWindows_ParaSet(void)
{
	      u8    key = 0 ;
	      u8    CurrentSelIndex = 1;
	      u8    FanHuiKeyCount = 0 ;
	
	      HighLightItem	= 0 ;
        MenuWindows_ParaSetMenu1();
	
        Item2_ItemLen = 4 ;
				
				KeyActiveStru.KeyType = 0xFF ;
				KeyActiveStru.KeyOperate = 0xFF ;
				
	      while(1)
				{
					
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
						if(Dmr400M_DataReceFlag==1)    
						{		     
										 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
										 Dmr400MReceFrame[3]  = 0xFF ;
										 Dmr400MReceFrame[9]  = 0xFF ;
										 Dmr400M_DataReceFlag = 0x00 ;
						}
						
		        HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
			    	if(KeyActiveStru.KeyOperate!=0xFF)	
						{							
							     if(KeyActiveStru.KeyOperate==0x02)
								   {			
										 
													key = CovertKeyValue(KeyActiveStru.KeyType) ;
											      
										      AudioSpkOpen();	
										 
										      printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;

										 			if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)               //返回
													{
														     wtn_play_voice_one(50);
														     if(FanHuiKeyCount>0)
																 {
														          FanHuiKeyCount-- ;
																	    MenuWindows_ParaSetMenu1();			
																 }	
                                 else
																 {																	 
																	     return ;	 
																 } 
													}
										 			else if(key>=KEY_SHUZI1_VALUE&&key<=KEY_SHUZI7_VALUE)
												  {
														       wtn_play_voice(key);
														       if(key == KEY_SHUZI1_VALUE)
																	 {
																	      	MenuWindows_SetTime() ;   																 
																	 }
																	 else if(key==KEY_SHUZI2_VALUE)
																	 {
	                                        SendTime = MenuWindows_SetAPN() ; 
																	 }
																	 else if(key==KEY_SHUZI6_VALUE)
																	 {
	                                        MenuWindows_SetPort(); 		
																	 }
                                   else
																	 {
																		       MenuWindows_SetIP(key-1); 					 
																	 }
																	 
																	 MenuWindows_ParaSetMenu1();	
																	 
																	 FanHuiKeyCount = 1 ;		
																	 
	 
												  }
													else if(key==KEY_QUEREN_VALUE||key==KEY_RIGHT_VALUE)               //返回
													{
														       wtn_play_voice_one(83);
																	 if(CurrentSelIndex==KEY_SHUZI1_VALUE)
																	 {	 
																	        MenuWindows_SetTime() ;      
																	 }
																	 else if(CurrentSelIndex==KEY_SHUZI2_VALUE)
																	 {
																		     MenuWindows_SetAPN() ; 
																	 }
																	 else if(CurrentSelIndex==KEY_SHUZI6_VALUE)
																	 {
																		        MenuWindows_SetPort() ;    															 
																	 }
                                   else
																	 { 
																					 MenuWindows_SetIP(CurrentSelIndex-1);
                                           HighLightItem	= CurrentSelIndex ; 																		 
																					 			
																	 }
																	 
																	 MenuWindows_ParaSetMenu1();
																	 FanHuiKeyCount = 1 ;			 								 
													}	    
												 else if(key==KEY_CHAXUN_VALUE)               //发送命令
												 {	    											      
                                    wtn_play_voice_one(28);
												 }
												 else if(key==KEY_JING_VALUE)
												 {
                                   wtn_play_voice_one(28);
												 }
//											 else if(key==KEY_LEFT_VALUE)
//											 {
//													       wtn_play_voice_one(28);
//																	if(DisMenuFlag == 1)
//																	{
//																					CurrentSelIndex = 7 ;
//																					HighLightItem = 1 ;
//																					MenuWindows_ParaSetMenu2();
//																					DisMenuFlag = 2 ;
//																	}
//																	else if(DisMenuFlag == 2)
//																	{
//																					CurrentSelIndex = 1 ;
//																					HighLightItem = 1 ;
//																					MenuWindows_ParaSetMenu1();		
//																					DisMenuFlag = 1 ;
//																	}
//													
//												}
//												else if(key==KEY_RIGHT_VALUE)
//												{
//													           wtn_play_voice_one(28);
//																		if(DisMenuFlag == 1)
//																		{
//																			      CurrentSelIndex = 7 ;
//																			      HighLightItem = 1 ;
//																						MenuWindows_ParaSetMenu2();
//																						DisMenuFlag = 2 ;
//																		}
//																		else if(DisMenuFlag == 2)
//																		{
//																			      CurrentSelIndex = 1 ;
//																			      HighLightItem = 1 ;
//																						MenuWindows_ParaSetMenu1();		
//																						DisMenuFlag = 1 ;
//																		}
//														
//											}
												if(key==KEY_UP_VALUE)  
												{	
																wtn_play_voice_one(28);
																if(CurrentSelIndex>1)
																		 CurrentSelIndex-- ;
																else
																		 CurrentSelIndex = 7 ;
																
																 if(CurrentSelIndex==7)
																 {
																			HighLightItem = 1 ;
																			MenuWindows_ParaSetMenu2();	
																 }
																 else
																 {
																			HighLightItem = CurrentSelIndex ;
																			MenuWindows_ParaSetMenu1();				
																 }
												}
												if(key==KEY_DOWN_VALUE)  
												{
													    wtn_play_voice_one(28);
														if(CurrentSelIndex<7)
																 CurrentSelIndex++ ;
														else
																 CurrentSelIndex =1 ;
																	
														 if(CurrentSelIndex==7)
														 {
														      HighLightItem = 1 ;
															    MenuWindows_ParaSetMenu2();	
														 }
														 else
														 {
														      HighLightItem = CurrentSelIndex ;
															    MenuWindows_ParaSetMenu1();		
														 }
														 
												}												 
												else if(key==KEY_XING_VALUE)
												{
														 wtn_play_voice_one(28);
												}	


													KeyActiveStru.KeyType = 0xFF ;											 
						        }
									 
									   KeyActiveStru.KeyOperate = 0xFF ; 	 
            } 
						
						if(T1S_Flag==1)
						{  
									RUN_LED__TURN ;
							
									IWDG_ReloadCounter();    //喂狗
							
						}						 					
			 }
}
bool MenuWindows_GSMRStatus(u16 FengYaValue,u32 ReceTime)
{
	          u8  Temp[4] = {0,0,0,0};
	
						sprintf(DisTemp,"%s","                 ") ;
	
						Item1_ItemLen = 0 ;
						Item2_ItemLen = 0 ;
						Item3_ItemLen = 0 ;
						Item4_ItemLen = 0 ;
						Item5_ItemLen = 0 ;
						Item6_ItemLen = 0 ;

						Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text) ;
						Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text) ;
						Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text) ;		
						Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text) ;
						Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text) ;

	          Temp[0] = (ReceTime>>24)&0xFF ;
						Temp[1] = (ReceTime>>16)&0xFF ;
						Temp[2] = (ReceTime>>8)&0xFF ;
						Temp[3] = (ReceTime)&0xFF ;
						
						Item2_ItemLen = 0 ;
	          sprintf(DisTemp,"%s","操作");						
						
						Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
		         
	          Item2_Text[4] = 0x1A ;
	          Item2_Text[5] = 0xFF ;

						Item2_ItemLen = 3 ;		
						sprintf(DisTemp,"查询应答");
						Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
						
						
	          Item3_ItemLen = 0 ;
	          sprintf(DisTemp,"%s","模块");						
						
						Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
		         
	          Item3_Text[4] = 0x1A ;
	          Item3_Text[5] = 0xFF ;
	
	          Item3_ItemLen = 3 ;
						
						Item4_ItemLen = 0 ;
	          sprintf(DisTemp,"%s","自检");						
						
						Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
		         
	          Item4_Text[4] = 0x1A ;
	          Item4_Text[5] = 0xFF ;
	
	          Item4_ItemLen = 3 ;
						
						
						if((FengYaValue&0x0C)==0x00)
						{	
											Item3_ItemLen = 3 ;
											sprintf(DisTemp,"%s","已配置");
											Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
								
											if((FengYaValue&0x03)==0)
											{
															sprintf(DisTemp,"%s","正常");
															Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2);	
											}
											else if((FengYaValue&0x03)==1)
											{
															sprintf(DisTemp,"%s","故障");
															Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
											}	
						}	
						else
						{	
											sprintf(DisTemp,"%s","未配置");
											Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;													
						}

						Item5_ItemLen = 0 ;
	          sprintf(DisTemp,"%s","状态");						
						Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
		         
	          Item5_Text[4] = 0x1A ;
	          Item5_Text[5] = 0xFF ;
						
						Item5_ItemLen = 3 ;
						
//						sprintf(DisTemp," %02X ",Temp[1]);	
//						Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
					
							if((Temp[1]&0x80)==0)
							{		
											 sprintf(DisTemp,"%s","卡故障");			
							}
							else
							{
											if((Temp[1]&0x0F)==0)
											{		
															sprintf(DisTemp,"%s","未注册");			
											}
											else if((Temp[1]&0x0F)==1)
											{
															sprintf(DisTemp,"%s","注册成功");								
											}						
											else if((Temp[1]&0x0F)==2)
											{
															sprintf(DisTemp,"%s","已注册");		
											}
						 }
					
					
				 	  Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;
							
						Item6_ItemLen = 0 ;
	          sprintf(DisTemp,"%s","场强");						
						Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
	          Item6_Text[4] = 0x1A ;
	          Item6_Text[5] = 0xFF ;
						Item6_ItemLen = 3 ;
						
						sprintf(DisTemp," %d ",Temp[0]) ;
						Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
							
}
void MenuWindows_QuryGSMRMenu(void)
{
					MenuWindows_Refresh();
				
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

          sprintf(DisTemp,"%s","G");		
	
				 TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				
					sprintf(DisTemp,"%s","网状态");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			

					Item1_ItemLen = 0 ;
				
					sprintf(DisTemp,"%s","列尾");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
					sprintf(DisTemp,": %06X",IDNUM);		
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					Item2_ItemLen = 0 ;
				
					sprintf(DisTemp,"%s","操作");
					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
					sprintf(DisTemp,": ");		
          Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
					
					
					Item3_ItemLen = 0 ;
					sprintf(DisTemp,"%s","模块");
					Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
				
					sprintf(DisTemp,": ");		
					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
					
					
//				sprintf(DisTemp,"%s","已配置");
//				Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
					
					
					Item4_ItemLen = 0 ;
					sprintf(DisTemp,"%s","自检");
					Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
					
					sprintf(DisTemp,": ");		
					Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	

					
//				sprintf(DisTemp,"%s","正常");
//				Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
								
					Item5_ItemLen = 0 ;
					sprintf(DisTemp,"%s","状态");
					Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
					
					sprintf(DisTemp,": ");		
					Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
					
//				sprintf(DisTemp,"%s","已注册");
//				Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
					
					
					Item6_ItemLen = 0 ;
					Item6_Text[0] = 0x3A ;
					Item6_Text[1] = 0x57 ;	
					Item6_Text[2] = 0x3A ;
					Item6_Text[3] = 0x5F ;		 
					Item6_Text[4] = 0x1A ;
					Item6_Text[5] = 0xFF ;
					
					Item6_ItemLen = 3 ;
					
//					sprintf(DisTemp,"%s"," 55dBm");
//					Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
					

					HighLightItem	= 0 ;
					Button1Value =  0x0000 ;
					Button2Value =  0x0001 ;
					
					StartWaitQueRenFlag = WaitQueRen_SetIdnumPara ;
					
					MenuWindowsDis(WindowID);	

}
void MenuWindows_QuryGSMR(void)
{
	      u8    key = 0 ;
	
        MenuWindows_QuryGSMRMenu();
	
				KeyActiveStru.KeyType = 0xFF ;
				KeyActiveStru.KeyOperate = 0xFF ;
				
	      while(1)
				{
					
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
						if(Dmr400M_DataReceFlag==1)    
						{		     
									 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
	 
									 Dmr400MReceFrame[3]  = 0xFF ;
									 Dmr400MReceFrame[9]  = 0xFF ;
									 Dmr400M_DataReceFlag = 0x00 ;
						}
						
		        HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
						if(Dmr400M_DataReceFlag==1)    
						{		     
									 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
	 
									 Dmr400MReceFrame[3]  = 0xFF ;
									 Dmr400MReceFrame[9]  = 0xFF ;
									 Dmr400M_DataReceFlag = 0x00 ;
						}
			    	if(KeyActiveStru.KeyOperate!=0xFF)	
						{							
							     if(KeyActiveStru.KeyOperate==0x02)
								   {				    
													key = CovertKeyValue(KeyActiveStru.KeyType) ;
											      
										      AudioSpkOpen();	
										 
										      printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
										 
                          if(key == KEY_CHAXUN_VALUE||key==KEY_QUEREN_VALUE)               //发送命令
													{	    
														 				Button1Value = 0x0000 ;
																		Button2Value = 0x0001 ;

														        Item2_ItemLen = 3 ;
																		sprintf(DisTemp,"%s","查询");
																		Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	

																		sprintf(DisTemp,"%s","G");		
																		Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
																		
																		sprintf(DisTemp,"%s","网");
																		Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
     
																		MenuWindowsDis(0xFFFFFFFF);	
														
																		Dmr400M_DataSendDataFun(0xD7) ;
													}		
													else if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)               //返回
													{
														      wtn_play_voice_one(50);
																  return ;
													}
													else
													{
													        wtn_play_voice_one(28); 
													}
													
												
													KeyActiveStru.KeyType = 0xFF ;											 
						        }
									 
									   KeyActiveStru.KeyOperate = 0xFF ; 	 
            } 
						
						if(T1S_Flag==1)
						{  
									RUN_LED__TURN ;
									IWDG_ReloadCounter();    //喂狗
							
						}						 					
			 }
}
void MenuWindows_QuryMoudleMenu(void)
{

					MenuWindows_Refresh();
				
					TitleTextLen = 0 ; 
				
					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				
					sprintf(DisTemp,"%s","移动模块");	
					TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				

					sprintf(DisTemp,"%s","=====");	
					TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			

					Item1_ItemLen = 0 ;
					sprintf(DisTemp,"%s","列尾");
					Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
					sprintf(DisTemp,": %06X",IDNUM);		
					Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
					
					Item2_ItemLen = 0 ;
					sprintf(DisTemp,"%s","操作");
					Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
					sprintf(DisTemp,": ");		
					Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	

	
					Item3_ItemLen = 0 ;
					sprintf(DisTemp,"%s","模块");
					Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
				
					sprintf(DisTemp,": ");		
					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
					

					Item4_ItemLen = 0 ;
					sprintf(DisTemp,"%s","状态");
					Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
					
					sprintf(DisTemp,": ");		
					Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
					
					
					Item5_ItemLen = 0 ;
					sprintf(DisTemp,"%s","卫星");
					Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;

					sprintf(DisTemp,": ");		
					Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
			
					Item6_ItemLen = 0 ;
					sprintf(DisTemp,"%s","HDOP:    STA:  ");
					Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
				

					HighLightItem	= 0 ;
					Button1Value =  0x0000 ;
					Button2Value =  0x0001 ;
					
					StartWaitQueRenFlag = WaitQueRen_SetIdnumPara ;
					
					MenuWindowsDis(WindowID);	

}

void MenuWindows_QuryMoudle(void)
{
	      u8    key = 0 ;
	
        MenuWindows_QuryMoudleMenu();
	
        Item2_ItemLen = 4 ;
				
				KeyActiveStru.KeyType = 0xFF ;
				
				KeyActiveStru.KeyOperate = 0xFF ;
				
	      while(1)
				{
		        HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
			    	if(KeyActiveStru.KeyOperate!=0xFF)	
						{							
							     if(KeyActiveStru.KeyOperate==0x02)
								   {				    
													key = CovertKeyValue(KeyActiveStru.KeyType) ;
											      
										      AudioSpkOpen();	
										 
										      printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;

													 if(key==KEY_CHAXUN_VALUE)               //发送命令
													 {	    
																		Button2Value = 0x0001 ;
														 
														        Item2_ItemLen = 3 ;
														 
																		sprintf(DisTemp,"查询模块");
																		Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
														 

																		MenuWindowsDis(0xFFFFFFFF);	
														 
																		Dmr400M_DataSendDataFun(0xD5) ;
													}
													else if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)               //返回
													{  
														     wtn_play_voice_one(50);
														
																 return ;
													}
													else
													{
													        wtn_play_voice_one(28); 
													}

													KeyActiveStru.KeyType = 0xFF ;											 
						        }
									 
									   KeyActiveStru.KeyOperate = 0xFF ; 	 
            } 	
						if(Dmr400M_DataReceFlag==1&&CurrentWorkMode==WorkMode_ShuZi)    
						{		
								//		 if(Dmr400M_ReceCommandFlag==0)                  //短时间内不处理
										 {
													 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
													 Dmr400MReceFrame[3] = 0x00 ;
													 Dmr400M_DataReceFlag = 0 ;
										 }
										 
										 Dmr400MReceFrame[3]  = 0xFF ;
										 Dmr400MReceFrame[9]  = 0xFF ;
										 Dmr400M_DataReceFlag = 0x00 ;
						}
						
						
						if(T1S_Flag==1)
						{  
									RUN_LED__TURN ;
									IWDG_ReloadCounter();    //喂狗
						}						 					
			 }
}

void MenuWindows_SetIDParaMenu(void)
{
				MenuWindows_Refresh();
			
				TitleTextLen = 0 ; 
			
				sprintf(DisTemp,"%s","=====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			
				sprintf(DisTemp,"%s","ID");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			
				sprintf(DisTemp,"设置");

				TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
			
				sprintf(DisTemp,"%s","=====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
			
				Item1_ItemLen = 0 ;
			
				sprintf(DisTemp,"主机");
				Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
			
				sprintf(DisTemp,"%s",": ");		
				Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
				if(IDNUM==0x000000||IDNUM==0xFFFFFF)
				{
								sprintf(DisTemp,"%s","******");
								Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
								HighLightItem	= 1 ;
					
				}
				else
				{
								sprintf(DisTemp,"%06X",IDNUM);
								Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
					
								HighLightItem	= 3 ;
				}

			  Item2_ItemLen = 0 ;
			
				sprintf(DisTemp,"操作");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
			
				sprintf(DisTemp,"%s",": ");		
				Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;

				
			  Item3_ItemLen = 0 ;
			
				sprintf(DisTemp,"参数");
				Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
			
				sprintf(DisTemp,":");		
				Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
				
				sprintf(DisTemp,"%s","######");
				Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
				
				Button1Value =  0x0001 ;
				Button2Value =  0x0001 ;
				
				
				StartWaitQueRenFlag = WaitQueRen_SetIdnumPara ;
				MenuWindowsDis(WindowID);	
}


void MenuWindows_SetIDPara(void)
{
	      u8    key = 0 ;
	      u32   SetValueTemp = 0 ;
	      u8    SetIdnumIndex = 0 ;
	      u8    SetIdnumTemp[7] ;
	      u8    CurrentSelIndex = 0 ;

        MenuWindows_SetIDParaMenu();
				
	      CurrentSelIndex = HighLightItem ;
				
				KeyActiveStru.KeyType = 0xFF ;		
				KeyActiveStru.KeyOperate = 0xFF ;

	      while(1)
				{
					
		        HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
						if(Dmr400M_DataReceFlag==1)    
						{		     
									 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
				  				 Dmr400MReceFrame[3]  = 0xFF ;
									 Dmr400MReceFrame[9]  = 0xFF ;
									 Dmr400M_DataReceFlag = 0x00 ;
						}
						
						HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
			    	if(KeyActiveStru.KeyOperate!=0xFF)	
						{							
							     if(KeyActiveStru.KeyOperate==0x02)
								   {				    
													key = CovertKeyValue(KeyActiveStru.KeyType) ;
											      
										      AudioSpkOpen();	
										 
										      printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
										 
										 			if(key>=KEY_SHUZI0_VALUE&&key<=KEY_SHUZI9_VALUE)
												  {
														  			 if(StartWaitQueRenFlag==WaitQueRen_SetIdnum)
																		 {
																						 wtn_play_voice(key);
																						 SetValueTemp = SetValueTemp<<4;
																						 SetValueTemp |= key ;
																						 SetIdnumTemp[SetIdnumIndex++] = 0x30 + key ;
																						 if(SetIdnumIndex>6)
																						 {
																									SetIdnumIndex = 0 ;
																									SetValueTemp 	= 0 ;													 
																						 }

																						 Item1_ItemLen = 3 ; 
																						 sprintf(DisTemp,"%01X",SetValueTemp) ;
																						 Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2)  ;
																						 

																						 MenuWindowsDis(0xFFFFFFFF);
																						 
																						 SpkOpenCount = 10;				
																		 }
																		 else if(StartWaitQueRenFlag==WaitQueRen_SetIdnumPara)
																		 {
																						 wtn_play_voice(key);
																						 SetValueTemp = SetValueTemp<<4;
																						 SetValueTemp |= key ;
																						 SetIdnumTemp[SetIdnumIndex++] = 0x30 + key ;
																						 if(SetIdnumIndex>6)
																						 {
																									SetIdnumIndex = 0 ;
																									SetValueTemp 	= 0 ;													 
																						 }

																						 Item3_ItemLen = 3 ;
																						 sprintf(DisTemp,"%01X",SetValueTemp) ;
																						 
																						 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
																						 
																						 MenuWindowsDis(0xFFFFFFFF);
																						 
																						 SpkOpenCount = 10;				
																		 }
   
												  }
													else if(key==KEY_QUEREN_VALUE)               //确认
													{
																	if(StartWaitQueRenFlag==WaitQueRen_SetIdnum)
																	{
																					if(SetIdnumIndex==6)
																					{
																								 wtn_play_voice_one(91) ;
																						
																								 StartWaitQueRenFlag = 0 ;
																								 BackLinghtCount = 15 ;
																								 SetIdnumIndex = 0 ;
																						
																								 IDNUM = SetValueTemp ;
																						
																								 HighLightItem = 0 ;	 
																								 Button1Value = 0x0000 ;
																								 Button2Value = 0x0001 ;		
			
																						     MenuWindowsDis(0xFFFFFFFF);	
																					}
																					else
																					{         
																									wtn_play_voice_one(133) ;
																					}
																							
																	}
																  else if(StartWaitQueRenFlag==WaitQueRen_SetIdnumPara)
																	{
																					if(SetIdnumIndex==6)
																					{
																								 wtn_play_voice_one(91) ;
																						
																								 StartWaitQueRenFlag = 0 ;
																								 BackLinghtCount = 15 ;
																								 SetIdnumIndex = 0 ;
																						
																						
																								 EditIDNUM = SetValueTemp ;
																						
																							   sprintf(DisTemp,"%06X",EditIDNUM);
																						
																						     Item3_ItemLen = 3 ;
								                                 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
																						
																								 HighLightItem = 0 ;	
																								 Button1Value = 0x0000 ;
																								 Button2Value = 0x0001 ;		
			
																						     MenuWindowsDis(0xFFFFFFFF);	
																					}
																					else
																					{         
																									wtn_play_voice_one(133) ;
																					}				
																	}
													 }
													 else if(key== KEY_CHAXUN_VALUE)               //发送命令
													 {	    
														 				Button1Value = 0x0000 ;
																		Button2Value = 0x0001 ;
														 
														        Item2_ItemLen = 3 ;
																		sprintf(DisTemp,"号码设置") ;
																		Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	

														 
														        SendTime = EditIDNUM  ;	
																		MenuWindowsDis(0xFFFFFFFF);		
																		Dmr400M_DataSendDataFun(0xD3) ;	
													 }
													 else if(key==KEY_UP_VALUE||key==KEY_DOWN_VALUE)               //返回
													 {
																		if(CurrentSelIndex == 1)
																		{	
																					 CurrentSelIndex = 2 ;
																					 HighLightItem	= 3 ;
																					 key = KEY_JING_VALUE ;
																		}
																		else
																		{	
																					 CurrentSelIndex = 1 ;
																					 HighLightItem	= 1 ;
																					 key = KEY_XING_VALUE ;
																		}
																		wtn_play_voice_one(28);
													 }
													 
													 else if(key==KEY_JING_VALUE)
													 {
																		StartWaitQueRenFlag = WaitQueRen_SetIdnumPara ;  
																		StartWaitQueRenCount = 30;
																		BackLinghtCount = 30 ;
																		SetIdnumIndex = 0 ;
																		SetValueTemp = 0 ;	
																		
																		LinkStatus = 0 ;	
														        HighLightItem	= 3 ;
																		Button1Value =  0x0001 ;
																		Button2Value =  0x0002 ;
														       
																		sprintf(DisTemp,"%s","###### ");
														        Item3_ItemLen = 3 ;
																		Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);
														
																		MenuWindowsDis(0xFFFFFFFF) ;
														
																		wtn_play_voice(0xA0) ;
														
													}		
													else if(key==KEY_XING_VALUE)
													{
																		StartWaitQueRenFlag = WaitQueRen_SetIdnum ;  
																		StartWaitQueRenCount = 30;
																		BackLinghtCount = 30 ;
																		SetIdnumIndex = 0 ;
																		SetValueTemp = 0 ;	
																		
																		LinkStatus = 0 ;
														
														        HighLightItem	= 1 ;
																		Button1Value =  0x0001 ;
																		Button2Value =  0x0002 ;
														       
																		sprintf(DisTemp,"%s","****** ");
														
														        Item1_ItemLen = 3 ;
																		Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2);
														
																		MenuWindowsDis(0xFFFFFFFF) ;
														
																		wtn_play_voice(0xA0) ;
													}	
													else if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)               //返回
													{		
														   	wtn_play_voice_one(50);              //返回音
																 return ;
													}
													else
													{
													        wtn_play_voice_one(28); 
													}
												
													KeyActiveStru.KeyType = 0xFF ;											 
						        }
									 
									   KeyActiveStru.KeyOperate = 0xFF ; 	 
            } 
						
						if(Dmr400M_DataReceFlag==1)    
						{		 
							
									 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
	 
									 Dmr400MReceFrame[3]  = 0xFF ;
									 Dmr400MReceFrame[9]  = 0xFF ;
									 Dmr400M_DataReceFlag = 0x00 ;
						}
						
						if(T1S_Flag==1)
						{  
									RUN_LED__TURN ;
									IWDG_ReloadCounter();    //喂狗
						}						 					
			 }
}

u32 MenuWindows_SetAPN(void)
{
	      u8    Data[2] ={0,0};				
				char  Index = 0 ;
        u8    key = 0 ;
	      u32   RetValue = 0;	 
	        
				MenuWindows_Refresh();

				TitleTextLen = 0 ;

				sprintf(DisTemp,"%s","====APN");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
				sprintf(DisTemp,"设置");
				TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
				
				sprintf(DisTemp,"%s","====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	

        Item1_ItemLen = 0 ;
				sprintf(DisTemp,"%s","列尾");
				Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
			
				sprintf(DisTemp,": %06X",IDNUM);		
				Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
				Item2_ItemLen = 0 ;
			
				sprintf(DisTemp,"%s","操作");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
			
				sprintf(DisTemp,": ");		
				Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
				sprintf(DisTemp,"%s","设置");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
				sprintf(DisTemp,"APN");		
				Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
				
				Item3_ItemLen = 0 ;
				sprintf(DisTemp,"%s","序号");
				Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
				
				sprintf(DisTemp,": ");		
				Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
			
				Item3_ItemLen =3 ;	
				sprintf(DisTemp,"**");		
				Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
	
				HighLightItem = 0x0003 ;
				Button1Value =  0x0001 ;
				Button2Value =  0x0001 ;
					
				MenuWindowsDis(WindowID) ;
				
				KeyActiveStru.KeyOperate = 0xFF	;	
				KeyActiveStru.KeyType = 0xFF	;
				
				StartWaitQueRenFlag = WaitQueRen_SetAPNPara ;  
					
			  while(1)
				{		
						   HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
							if(Dmr400M_DataReceFlag==1)    
							{		     
											 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
								
											 Dmr400MReceFrame[3]  = 0xFF ;
											 Dmr400MReceFrame[9]  = 0xFF ;
											 Dmr400M_DataReceFlag = 0x00 ;
							}
					
							 if(T1S_Flag==1)
							 {
										 RUN_LED__TURN ; 
										 IWDG_ReloadCounter();    //喂狗
										 T1S_Flag = 0 ;
							 }
							 
							 HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
						
							 if(KeyActiveStru.KeyOperate!=0xFF)	
							 {												 
												 if(KeyActiveStru.KeyOperate==0x02)
												 {				   
																		KeyActiveStru.KeyOperate = 0xFF ;	
													 
																		key = CovertKeyValue(KeyActiveStru.KeyType) ;

																		printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
													 
													          AudioSpkOpen();	

																		if(key>=KEY_SHUZI0_VALUE&&key<=KEY_SHUZI9_VALUE)
																		{
																			
																				      wtn_play_voice(key);
																			
																							if(StartWaitQueRenFlag==WaitQueRen_SetAPNPara) 
																							{
																										if(Index<2)
																										{
																												Data[Index] = key ;
																										}	
																										else
																										{
																												 Index = 0 ;
																										}
																									 if(Index == 0)
																														sprintf(DisTemp,"%01u*  ",Data[0]);	  
																										else if(Index == 1)
																													sprintf(DisTemp,"%01u%01u",Data[0],Data[1]);	
                                                    Item3_ItemLen = 3 ;
																										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
																										
																										Index++ ;
                                                   
                                                    MenuWindowsDis(0xFFFFFFFF) ;																										
																							}
																		}
																		else if(key==KEY_CHAXUN_VALUE)
																		{		
																			        SendCommandOprationIndex = 1 ;
																		        	SendTime = 0 ;
																			        SendTime |= RetValue<<24 ; /*获取时间*/	

																			        Dmr400M_DataSendDataFun(0xDA) ;				
																		}	
																	  else if(key==KEY_QUEREN_VALUE)
																		{		
																			    wtn_play_voice_one(83) ;
																			    HighLightItem = 0x0000 ;
																	     		StartWaitQueRenFlag = 0 ;
																			    RetValue = Data[0]*10 + Data[1]  ;
																					sprintf(DisTemp,"%02u",RetValue);
																										
                                          Item3_ItemLen = 3 ;
																					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
                                          MenuWindowsDis(0xFFFFFFFF) ;	
																			
																		}	
																		else if(key==KEY_XIAOHAO_VALUE)
																		{		
																					if(StartWaitQueRenFlag==WaitQueRen_SetAPNPara) 	
																					{
																										wtn_play_voice_one(82) ;
																										Item3_ItemLen =3 ;
																										StartWaitQueRenFlag = 0 ;
																						
																										sprintf(DisTemp,"**");		
																										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
																							
																										HighLightItem = 0x0000 ;
																										Button1Value =  0x0001 ;
																										Button2Value =  0x0001 ;
																											
																										MenuWindowsDis(WindowID) ;
																					}
																					else
																					{
																										wtn_play_voice_one(50) ;
																										return  ;
																					}
																		}
																	  else if(key==KEY_LEFT_VALUE)
																		{		
																						
																							wtn_play_voice_one(50) ;
																							return  ;
																						
																		}
							                      if(key==KEY_XING_VALUE||key==KEY_JING_VALUE)
																		{				
																							StartWaitQueRenFlag = WaitQueRen_SetAPNPara ;  
																							StartWaitQueRenCount = 30;
																							BackLinghtCount = 30 ;
																							Index = 0 ;
																			        Data[0] = 0 ;
																			        Data[1] = 0 ;
																							
																							LinkStatus = 0 ;	
																							HighLightItem	= 3 ;
																							Button1Value =  0x0001 ;
																							Button2Value =  0x0002 ;
																			
																							Item3_ItemLen = 3 ;	
																						  sprintf(DisTemp,"%s","**");
																							Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);
																			
																							MenuWindowsDis(0xFFFFFFFF) ;
																							wtn_play_voice_one(91) ; ;
																		}
																		else
																		{

																	  }	
												 }		
						    }	
					 }						 						
}

u32 MenuWindows_PassWord(void)
{
	      u8    Data[4] = {0,0,0,0};				
				u8    Index = 0 ;
        u8    key = 0 ;
	      u32   PortValue = 0;	 
	        
				MenuWindows_Refresh();

				TitleTextLen = 0 ;

				sprintf(DisTemp,"%s","====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
				sprintf(DisTemp,"密码输入");
				TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
				
				sprintf(DisTemp,"%s","====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	

        Item1_ItemLen = 0 ;
				sprintf(DisTemp,"%s","密码");
				Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
			
				sprintf(DisTemp,": ");		
				Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;

        Item1_ItemLen = 4 ;
				
				sprintf(DisTemp,"****");		
				Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
				

				HighLightItem = 0x0001 ;
				Button1Value =  0x0000 ;
				Button2Value =  0x0001 ;
					
				MenuWindowsDis(WindowID) ;
				
				KeyActiveStru.KeyOperate = 0xFF	;	
				KeyActiveStru.KeyType = 0xFF	;
				
				StartWaitQueRenFlag = WaitQueRen_SetPORTPara ;

        HighLightItem	= 1;
					
			  while(1)
				{
					
								HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
							 if(T1S_Flag==1)
							 {
										 RUN_LED__TURN ;
										 IWDG_ReloadCounter();    //喂狗
										 T1S_Flag = 0 ;
							 }
							 
							 HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
						
							 if(KeyActiveStru.KeyOperate!=0xFF)	
							 {												 
												 if(KeyActiveStru.KeyOperate==0x02)
												 {				   
													 
																		KeyActiveStru.KeyOperate = 0xFF ;	
													 
																		key = CovertKeyValue(KeyActiveStru.KeyType) ;

																		printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
													 
													          AudioSpkOpen();	
													 
																		if(key>=KEY_SHUZI0_VALUE&&key<=KEY_SHUZI9_VALUE)
																		{

																				     wtn_play_voice_one(28);
																			
																						 if(StartWaitQueRenFlag==WaitQueRen_SetPORTPara) 
																							{
																										if(Index<4)
																										{
																												Data[Index] = key ;
																										}
																										
																									 if(Index == 0)
																													sprintf(DisTemp,"%01u***",Data[0]);	  
																									 else if(Index == 1)
																													sprintf(DisTemp,"*%01u**",Data[1]);
																									 else  if(Index == 2)
																													sprintf(DisTemp,"**%01u*",Data[2]);	  
																										else if(Index == 3)
																													sprintf(DisTemp,"***%01u",Data[3]);																											
                                                  
																										Item1_ItemLen = 4 ;
																										Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
																										
																										Index++ ;

	                                                  if(Index==4)
																									  {
																			
																															 PortValue = Data[0]*1000 + Data[1]*100 + Data[2]*10 + Data[3]  ;
																											
																											         if(PortValue==1234||PortValue==3421)
																															 {
																															     	  wtn_play_voice_one(104);
																															        return   1 ; 
																															 
																															 }
																															 else if(PortValue==9876||PortValue==6789)
																															 {
																															     	  wtn_play_voice_one(104);
																															        return   2 ; 
																															 
																															 }
																															 else
																															 {
																																      wtn_play_voice_one(92);			  
																																      Index = 0 ;
																																      Item1_ItemLen = 3 ;
																																			sprintf(DisTemp,"****");		
																																			Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
																															       
																															 }
																									  }
																										
																										MenuWindowsDis(WindowID) ;	
																										
																							}
																		}
																		else if(key==KEY_XIAOHAO_VALUE)
																		{			
																							wtn_play_voice_one(50);
																							return 0 ;			
																		}
																		else
																		{
																				 wtn_play_voice_one(28);
																	  }	
												 }		
						    }	
					 }						 						
}
u32 MenuWindows_SetPort(void)
{
	      u8    Data[4] = {0,0,0,0};				
				char  Index = 0 ;
        u8    key = 0 ;
	      u32   PortValue = 0;	 
	        
				MenuWindows_Refresh();

				TitleTextLen = 0 ;

				sprintf(DisTemp,"%s","====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
				sprintf(DisTemp,"端口设置");
				TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
				
				sprintf(DisTemp,"%s","====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	

        Item1_ItemLen = 0 ;
				sprintf(DisTemp,"%s","列尾");
				Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
			
				sprintf(DisTemp,": %06X",IDNUM);		
				Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
				Item2_ItemLen = 0 ;
			
				sprintf(DisTemp,"%s","操作");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
			
				sprintf(DisTemp,": ");		
				Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
				sprintf(DisTemp,"%s","端口设置");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				

				Item3_ItemLen = 0 ;
				sprintf(DisTemp,"%s","端口");
				Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
				
				sprintf(DisTemp,":");		
				Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
			
				Item3_ItemLen =3 ;	
				sprintf(DisTemp,"****");		
				Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
	
				HighLightItem = 0x0003 ;
				Button1Value =  0x0001 ;
				Button2Value =  0x0001 ;
					
				MenuWindowsDis(WindowID) ;
				
				KeyActiveStru.KeyOperate = 0xFF	;	
				KeyActiveStru.KeyType = 0xFF	;
				
				StartWaitQueRenFlag = WaitQueRen_SetPORTPara ;

        HighLightItem	= 3 ;
					
			  while(1)
				{
					
								HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
								if(Dmr400M_DataReceFlag==1)    
								{		     
											 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
			 
											 Dmr400MReceFrame[3]  = 0xFF ;
											 Dmr400MReceFrame[9]  = 0xFF ;
											 Dmr400M_DataReceFlag = 0x00 ;
								}
						
							 if(T1S_Flag==1)
							 {
										 RUN_LED__TURN ;
								 
										 IWDG_ReloadCounter();    //喂狗
								 
										 T1S_Flag = 0 ;
							 }
							 
							 HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
						
							 if(KeyActiveStru.KeyOperate!=0xFF)	
							 {						
																	 
												 if(KeyActiveStru.KeyOperate==0x02)
												 {				   
													 
																		KeyActiveStru.KeyOperate = 0xFF ;	
													 
																		key = CovertKeyValue(KeyActiveStru.KeyType) ;

																		printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
													 
													          AudioSpkOpen();	
															 
																	
													 
																		if(key>=KEY_SHUZI0_VALUE&&key<=KEY_SHUZI9_VALUE)
																		{
																				     wtn_play_voice(key);
																			
																						 if(StartWaitQueRenFlag==WaitQueRen_SetPORTPara) 
																							{
																										if(Index<4)
																										{
																												Data[Index] = key ;
																										}	
																										else
																										{
																												 Index = 0 ;
																										}
																										
																									 if(Index == 0)
																														sprintf(DisTemp,"%01u***  ",Data[0]);	  
																									 else if(Index == 1)
																													sprintf(DisTemp,"%01u%01u**",Data[0],Data[1]);
																									 else  if(Index == 2)
																														sprintf(DisTemp,"%01u%01u%01u*",Data[0],Data[1],Data[2]);	  
																										else if(Index == 3)
																													sprintf(DisTemp,"%01u%01u%01u%01u",Data[0],Data[1],Data[2],Data[3]);																											
                                                  
																										Item3_ItemLen = 3 ;
																										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
																										
																										Index++ ;
                                                   
                                                    MenuWindowsDis(0xFFFFFFFF) ;																										
																							}
																		}
																		else if(key==KEY_QUEREN_VALUE)
																		{    	
																			        wtn_play_voice_one(83);
																			
																			
												                     PortValue = Data[0]*1000 + Data[1]*100 + Data[2]*10 + Data[3]  ;
																			
																			       StartWaitQueRenFlag = 0 ;
																			
																		      	 HighLightItem	= 2 ;
																			
																					   sprintf(DisTemp,"%s","端口设置");
				                                     Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
																			
																						 Item3_ItemLen = 3 ;	
																						 sprintf(DisTemp,"%d",PortValue);
																						 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);
																			
																			       MenuWindowsDis(0xFFFFFFFF) ;		
																		}
																		else if(key==KEY_CHAXUN_VALUE)
																		{		
																			        PortValue = Data[0]*1000 + Data[1]*100 + Data[2]*10 + Data[3]  ;
																			
																			        SendTime = PortValue<<16 ;
																			
																							SendCommandOprationIndex = 5 ;
																			
																							HighLightItem	= 2 ;
																			
																			        Item2_ItemLen = 3 ;
																					    sprintf(DisTemp,"%s","端口设置");
				                                      Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
																			
																							Item3_ItemLen = 3 ;	
																						  sprintf(DisTemp,"%04d",PortValue);
																						  Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);
																			
																				      MenuWindowsDis(0xFFFFFFFF) ;

																			        Dmr400M_DataSendDataFun(0xDA) ;
																		}	
																		else if(key==KEY_XIAOHAO_VALUE)
																		{		
																			        if(StartWaitQueRenFlag==WaitQueRen_SetPORTPara) 
																							{
																								     wtn_play_voice_one(82);
																								     StartWaitQueRenFlag = 0 ;
																										 PortValue = 0x00 ;
																										 Item1_ItemLen = 3 ;	
																										 Button1Value =  0x0001 ;
				                                             Button2Value =  0x0001 ;
																										 sprintf(DisTemp,"%s","**** ");
																										 Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);
																								     MenuWindowsDis(0xFFFFFFFF) ;
																								    
																							}
																			        else
																							{
																											wtn_play_voice_one(50);
																											return  ;
																							}
																							
																		}
																		else if(key==KEY_LEFT_VALUE)
																		{		

																						wtn_play_voice_one(50);
																						return  ;

																		}
							                      else if(key==KEY_XING_VALUE||key==KEY_JING_VALUE)
																		{				
																							StartWaitQueRenFlag = WaitQueRen_SetPORTPara ;  
																							StartWaitQueRenCount = 30;
																							BackLinghtCount = 30 ;
																							Index = 0 ;
																							
																							LinkStatus = 0 ;	
																							HighLightItem	= 3 ;
																							Button1Value =  0x0001 ;
																							Button2Value =  0x0002 ;
																			
																			        PortValue = 0 ;
																						 
																							Item3_ItemLen = 3 ;	
																						  sprintf(DisTemp,"%s","**** ");
																							Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);
																			
																							MenuWindowsDis(0xFFFFFFFF) ;
																		}
																		else
																		{
																				 wtn_play_voice_one(28);
																	  }	
												 }		
						    }	
					 }						 						
}

u32 MenuWindows_SetTime(void)
{
	      u8   Date[6] = {0} ;				
				u8   Time[6] = {0} ;
				char  Index = 0 ;
        u8    key = 0 ;
	        
				MenuWindows_Refresh();

				TitleTextLen = 0 ;

				sprintf(DisTemp,"%s","====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
				sprintf(DisTemp,"时间设置");
				TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
				
				sprintf(DisTemp,"%s","====");	
				TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	

        Item1_ItemLen = 0 ;
				sprintf(DisTemp,"%s","列尾");
				Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
			
				sprintf(DisTemp,": %06X",IDNUM);		
				Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
				Item2_ItemLen = 0 ;
			
				sprintf(DisTemp,"%s","操作");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
			
				sprintf(DisTemp,": ");		
				Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
				sprintf(DisTemp,"%s","设置时间");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
					

				GetRtcTimeData(); 
				
		    SendTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second ;    /*获取时间*/	


//				Date[0] = RTC_Year/10 ;
//				Date[1] = RTC_Year%10 ;
//				
//				Date[2] = RTC_Month/10 ;
//				Date[3] = RTC_Month%10 ;
//				
//				Date[4] = RTC_Date/10 ;
//				Date[5] = RTC_Date%10 ;
//				
//				
//				Time[0] = RTC_Hour/10 ;
//				Time[1] = RTC_Hour%10 ;
//				
//				Time[2] = RTC_Minute/10 ;
//				Time[3] = RTC_Minute%10 ;
//				
//				Time[4] = RTC_Second/10 ;
//				Time[5] = RTC_Second%10 ;

				Item3_ItemLen = 0 ;
				
				sprintf(DisTemp,"%s","日期");	
				Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
					
        sprintf(DisTemp,"%s",": ");		
				Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
				
//			sprintf(DisTemp,"%01u%01u:%01u%01u:%01u%01u",Date[0],Date[1],Date[2],Date[3],Date[4],Date[5]);		
//		  Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;

        sprintf(DisTemp,"**:**:**  ");		
        Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;  
				
				
				Item4_ItemLen = 0 ;
				sprintf(DisTemp,"%s","时间");
				Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
				
				sprintf(DisTemp,"%s",": ");		
				Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
											
//			sprintf(DisTemp,"%01u%01u:%01u%01u:%01u%01u",Time[0],Time[1],Time[2],Time[3],Time[4],Time[5]) ;		

        sprintf(DisTemp,"##:##:##  ");		
        Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
					
				HighLightItem = 0x0003 ;
				Button1Value =  0x0001 ;
				Button2Value =  0x0001 ;
					
				MenuWindowsDis(WindowID) ;

				KeyActiveStru.KeyOperate = 0xFF	;	
				
				KeyActiveStru.KeyType = 0xFF	;
				
				SendTime =0xFFFFFFFF	;
				
				StartWaitQueRenFlag = WaitQueRen_SetDatePara ;
					
			  while(1)
				{
							 HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
					
							 if(Dmr400M_DataReceFlag==1)    
							 {		     
											 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
											 Dmr400MReceFrame[3]  = 0xFF ;
											 Dmr400MReceFrame[9]  = 0xFF ;
											 Dmr400M_DataReceFlag = 0x00 ;
							 }
					
							 if(T1S_Flag==1)
							 {
										 RUN_LED__TURN ;
								 
										 IWDG_ReloadCounter();    //喂狗
								 
										 T1S_Flag = 0 ;
							 }
							 
							 HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
						
							 if(KeyActiveStru.KeyOperate!=0xFF)	
							 {						
																	 
												 if(KeyActiveStru.KeyOperate==0x02)
												 {				   
													 
																		KeyActiveStru.KeyOperate = 0xFF ;	
													 
																		key = CovertKeyValue(KeyActiveStru.KeyType) ;
																			
																		AudioSpkOpen();	
															 
																		printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
															 
																		if(key>=KEY_SHUZI0_VALUE&&key<=KEY_SHUZI9_VALUE)
																		{

																						if(StartWaitQueRenFlag==WaitQueRen_SetDatePara) 
																						{
																							      wtn_play_voice(key);
																							
																										if(Index<6)
																										{
																												 Date[Index] = key ;
																										}	
																										else
																										{
																											   Date[Index] = key ; 
																										}
																										
																									 if(Index == 0)
																														sprintf(DisTemp,"%01u*:**:**",Date[0]);	  
																										else if(Index == 1)
																													sprintf(DisTemp,"%01u%01u:**:**",Date[0],Date[1]);	 	
																										else if(Index == 2)
																													sprintf(DisTemp,"%01u%01u:%01u*:**",Date[0],Date[1],Date[2]);	 	
																										else if(Index == 3)
																													sprintf(DisTemp,"%01u%01u:%01u%01u:**",Date[0],Date[1],Date[2],Date[3]);	 	
																										else if(Index == 4)
																													sprintf(DisTemp,"%01u%01u:%01u%01u:%01u*",Date[0],Date[1],Date[2],Date[3],Date[4]);	 	
																										else if(Index == 5)
																													sprintf(DisTemp,"%01u%01u:%01u%01u:%01u%01u",Date[0],Date[1],Date[2],Date[3],Date[4],Date[5]);	 																											
																											
																										Item3_ItemLen = 3 ;
																										Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
																										
																										MenuWindowsDis(0xFFFFFFFF) ;	

                                                    Index++ ;																										
																						}
																						else if(StartWaitQueRenFlag==WaitQueRen_SetTimePara) 
																						{
																							      wtn_play_voice(key);
																							
																										if(Index<6)
																										{
																												Time[Index] = key ;
																										}
																										else
																										{
																										     Index = 0 ;
																											   Time[Index] = key ;
																										}	
																											
																									if(Index == 0)
																												sprintf(DisTemp,"%01u*:**:**",Time[0]);	  
																									else if(Index == 1)
																												sprintf(DisTemp,"%01u%01u:**:**",Time[0],Time[1]);	 	
																									else if(Index == 2)
																												sprintf(DisTemp,"%01u%01u:%01u*:**",Time[0],Time[1],Time[2]);	 	
																									else if(Index == 3)
																												sprintf(DisTemp,"%01u%01u:%01u%01u:**",Time[0],Time[1],Time[2],Time[3]);	 	
																									else if(Index == 4)
																												sprintf(DisTemp,"%01u%01u:%01u%01u:%01u*",Time[0],Time[1],Time[2],Time[3],Time[4]);	 	
																									else if(Index == 5)
																												sprintf(DisTemp,"%01u%01u:%01u%01u:%01u%01u",Time[0],Time[1],Time[2],Time[3],Time[4],Time[5]);	
																									
																									
																									Item4_ItemLen = 3 ;
																									Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
																									MenuWindowsDis(0xFFFFFFFF) ;		

                                                  Index++ ;																				
																						}
																	 }
																		
																	if(key==KEY_QUEREN_VALUE)
																	{    	
																							if(StartWaitQueRenFlag==WaitQueRen_SetDatePara) 
																							{
																											 RTC_Year =  Date[0]*10 + Date[1] ;
																											 RTC_Month = Date[2]*10 + Date[3] ;
																											 RTC_Date =  Date[4]*10 + Date[5] ;
																								
																								       Item3_ItemLen = 3 ;
																											 sprintf(DisTemp,"%02u:%02u:%02u",RTC_Year,RTC_Month,RTC_Date);		
				                                               Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;
																								
																								       HighLightItem = 0 ;
																								       wtn_play_voice_one(91) ;
																								       MenuWindowsDis(0xFFFFFFFF) ;
																							}
																							else if(StartWaitQueRenFlag==WaitQueRen_SetTimePara) 
																							{
																											 RTC_Hour =   Time[0]*10 + Time[1] ;
																											 RTC_Minute = Time[2]*10 + Time[3] ;
																											 RTC_Second = Time[4]*10 + Time[5] ;
																								
																											 Item4_ItemLen = 3 ;
																											 sprintf(DisTemp,"%02u:%02u:%02u",RTC_Hour,RTC_Minute,RTC_Second);		
				                                               Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
																								
																								       HighLightItem = 0 ;
																								       wtn_play_voice_one(91) ;
																								
																								       MenuWindowsDis(0xFFFFFFFF) ;					
																							}	       																		
																		}
																		else if(key==KEY_CHAXUN_VALUE)
																		{		
																							Item2_ItemLen = 3 ;			
																							sprintf(DisTemp,"%s","设置时间");
																							Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
																			  
																			        SendCommandOprationIndex = 0 ;
																			        SendTime = RTC_Year<<26|RTC_Month<<22|RTC_Date<<17|RTC_Hour<<12|RTC_Minute<<6|RTC_Second  ; /*获取时间*/	
																			
																			        Dmr400M_DataSendDataFun(0xDA);
																			
																		}	
																		else if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)
																		{		
																			        wtn_play_voice_one(50);
																					  	return  ;
																		}	
																		else if(key==KEY_XING_VALUE)
																		{		
																							wtn_play_voice_one(91);
																		          wtn_play_voice_one(95);
																							StartWaitQueRenFlag = WaitQueRen_SetDatePara ;  
																							StartWaitQueRenCount = 30;
																							BackLinghtCount = 30 ;
																							Index = 0 ;
																							
																							LinkStatus = 0 ;	

																							Item3_ItemLen = 3 ;
																							sprintf(DisTemp,"%s","**:**:** ");
																							Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);
																			
																							Item4_ItemLen = 3 ;
																							sprintf(DisTemp,"%02u:%02u:%02u",RTC_Hour,RTC_Minute,RTC_Second);		
				                                      Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
																			
																							 HighLightItem	= 3 ;
																							 Button1Value =  0x0001 ;
																							 Button2Value =  0x0002 ;
																							 
																							MenuWindowsDis(0xFFFFFFFF) ;
																			


																		}
																		else if(key==KEY_JING_VALUE)
																		{				
																							StartWaitQueRenFlag = WaitQueRen_SetTimePara ;  
																							StartWaitQueRenCount = 30;
																							BackLinghtCount = 30 ;
																							Index = 0 ;
																																						
																			        wtn_play_voice_one(91);
																		          wtn_play_voice_one(95);
																			
																							Item3_ItemLen = 3 ;
																							sprintf(DisTemp,"%02u:%02u:%02u",RTC_Year,RTC_Month,RTC_Date);		
				                                      Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;

																							Item4_ItemLen = 3 ;	
																						  sprintf(DisTemp,"%s","##:##:## ");
																							Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2);
			 
																		          HighLightItem	= 4 ;
																							Button1Value =  0x0001 ;
																							Button2Value =  0x0002 ;
																			
																							MenuWindowsDis(0xFFFFFFFF) ;

																			
																		}
																		else
																		{																				
	
																	 }	
												 }		
						    }	
					 }						 						
}
u32 MenuWindows_SetIP(u8 Funsel)
{
	
				u8    m = 0 ;
				char  DataTemp[17] ;
				u8    NewIp[4] ;	
				u32   temp = 0 ;															
				char  Temp[20];															
				char  Index = 0 ;
				char  j = 0,k = 0 ;
        u8    key = 0 ;
	      u32   SetIPTemp = 0x000000 ;
	
				MenuWindows_Refresh();

				TitleTextLen = 0 ;

			   if(Funsel==2)
				{
								sprintf(DisTemp,"%s","===");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
					
							  sprintf(DisTemp,"主用");
								TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
					
								sprintf(DisTemp,"%s","DNS IP==");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				 }
				
				else if(Funsel==3)
				{
								sprintf(DisTemp,"%s","===");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
					
							  sprintf(DisTemp,"备用");
								TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
					
								sprintf(DisTemp,"%s","DNS IP===");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	

				 }
				
				else if(Funsel==4)
				{
								sprintf(DisTemp,"%s","====");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
					
								sprintf(DisTemp,"%s","RMS IP");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;

								sprintf(DisTemp,"%s","====");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;						

				 }
				
				else if(Funsel==6)
				{
								sprintf(DisTemp,"%s","===");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
					
							  sprintf(DisTemp,"公网");
								TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
					
								sprintf(DisTemp,"%s","IP===");	
								TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
				}
				
				Item1_ItemLen = 0 ;
				sprintf(DisTemp,"%s","列尾");
				Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
			
				sprintf(DisTemp,": %06X",IDNUM);		
				Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
				
				Item2_ItemLen = 0 ;
			
				sprintf(DisTemp,"%s","操作");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
			
				sprintf(DisTemp,": ");		
				Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
				sprintf(DisTemp,"%s","设置");
				Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
				
				sprintf(DisTemp,"IP");		
				Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
					
				Item3_ItemLen = 0 ;

				Item1_ItemLen = 0 ;
				HighLightItem = 0x0003 ;
				Button1Value =  0x0001 ;
				Button2Value =  0x0001 ;
					
				MenuWindowsDis(WindowID);
				
				KeyActiveStru.KeyOperate =0xFF	;	
				KeyActiveStru.KeyType = 0xFF	;
				
			
			 while(1)
				{
										if(Dmr400M_DataReceFlag==1)    
										{		     
													 Dmr400M_DataReceDataFun(Dmr400MReceFrame+2);   //注意
													 Dmr400MReceFrame[3]  = 0xFF ;
													 Dmr400MReceFrame[9]  = 0xFF ;
													 Dmr400M_DataReceFlag = 0x00 ;
										}
										
										HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
										
										if(T1S_Flag==1)
										 {
													 RUN_LED__TURN ;
													 IWDG_ReloadCounter();    //喂狗	 
													 T1S_Flag = 0 ;
										 }
										 
										 HrnpSlaveRcvAndOperateCmdFromMaster() ;     // 接收处理函数	
									
										 if(KeyActiveStru.KeyOperate!=0xFF)	
										 {						
																				 
															 if(KeyActiveStru.KeyOperate==0x02)
															 {				   
																					KeyActiveStru.KeyOperate = 0xFF ;	
																 
																					key = CovertKeyValue(KeyActiveStru.KeyType) ;
																						
																					AudioSpkOpen();	
																		 
																					printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;
																		 
																					if(key>=KEY_SHUZI0_VALUE&&key<=KEY_SHUZI9_VALUE)
																					{
																						      wtn_play_voice(key) ;
																						
																									if(Index<15)
																									{
																											DataTemp[Index++] = key + 0x30;
																											DataTemp[Index] = '\0';	
																									}	
																					}
																					else if(key==KEY_RIGHT_VALUE||key==KEY_XING_VALUE)
																					{			
																									wtn_play_voice_one(28);																						
																									if(Index<12)
																									{
																											DataTemp[Index++] = '.';
																											DataTemp[Index] = '\0';	
																									}	       					
																					}
																					else if(key==KEY_LEFT_VALUE)
																					{
																						      wtn_play_voice_one(89);
																									if(Index>0)
																									{ 
																												 DataTemp[Index--] = '\0';
																									}	
																					}
																					else if(key==KEY_QUEREN_VALUE)
																					{    	
                                                wtn_play_voice_one(83);																						
																								for(j=0;j<17;j++)
																								{
																												if(DataTemp[j]=='.')
																												{
																														Temp[k]='\0';		
																														sscanf(Temp,"%u",&temp);
																														if(m < 4) { NewIp[m++] = temp ; }						
																														k = 0;																			
																														continue ;
																												}
																												if(DataTemp[j]=='\0')
																												{
																														Temp[k]='\0';		
																														sscanf(Temp,"%u",&temp);
																														if(m < 4) { NewIp[m] = temp ; }						
																														k = 0;	
																														break ;																			
																												}
																												
																												Temp[k++] = DataTemp[j];
																								}	
																								
																								/* 输入段数不足 4 段时，剩余字节补 0，避免下方拼装用到未初始化值 */
																								while(m < 4)
																								{
																									NewIp[m++] = 0 ;
																								}
																								SetIPTemp = NewIp[0]<<24|NewIp[1]<<16|NewIp[2]<<8|NewIp[3] ;
																								
																								SendTime = SetIPTemp ;
																								
																								HighLightItem = 0x0000 ;
																																													
																					}	
																					else if(key==KEY_CHAXUN_VALUE)
																					{		
																									Item2_ItemLen = 3 ;
																									sprintf(DisTemp,"%s","设置");
																									Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
																									
																									sprintf(DisTemp,"IP  ");		
																									Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
																									SendCommandOprationIndex = Funsel ;
				                                          SendTime = SetIPTemp ;
																						      HighLightItem = 0x0002 ;
																					       	MenuWindowsDis(WindowID);	
																			            Dmr400M_DataSendDataFun(0xDA);			
																					}
																					else if(key==KEY_XIAOHAO_VALUE)
																					{			
																						      wtn_play_voice_one(50);
																									return SetIPTemp ;
																					}
																					
																					sprintf(DisTemp,"%s",DataTemp);
																					Item3_ItemLen = 0 ;
																					Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
																					
																					MenuWindowsDis(WindowID);			
															 }		
									 }	
					 }						 
								
}
void MenuWindows_FunctionMenu(void)
{

	  MenuWindows_Refresh();
	
	  TitleTextLen = 0 ; 
	
	  sprintf(DisTemp,"%s","=====");	
    TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
		sprintf(DisTemp,"功能选择");

		TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
	
		sprintf(DisTemp,"%s","=====");	
    TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;	
	
	  Item1_ItemLen = 0 ;
	
	  sprintf(DisTemp,"%s","1.  ");	
    Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
	
		sprintf(DisTemp,"列尾");
		Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
	
		sprintf(DisTemp,"%s","ID");	
    Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;	
	
	  sprintf(DisTemp,"%s","设置");	
    Item1_ItemLen += HanZiCovertUniCode(DisTemp,Item1_Text+Item1_ItemLen*2) ;
	
	
	  Item2_ItemLen = 0 ;
	
	  sprintf(DisTemp,"%s","2.  ");	
    Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
	
		sprintf(DisTemp,"查询移动模块");
		Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
	
	  Item3_ItemLen = 0 ;
	
	  sprintf(DisTemp,"%s","3.  ");	
    Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
	
		sprintf(DisTemp,"查询");
		Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
	
		sprintf(DisTemp,"%s","G");	
    Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
	
	  sprintf(DisTemp,"%s","网状态");	
    Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
		
	  Item4_ItemLen = 0 ;
	  sprintf(DisTemp,"%s","4.  ");	
    Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
	
		sprintf(DisTemp,"参数查询");
		Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
		
	  Item5_ItemLen = 0 ;
	  sprintf(DisTemp,"%s","5.  ");	
    Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
	
		sprintf(DisTemp,"参数设置");
		Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
		
		Item6_ItemLen = 0 ;
	  sprintf(DisTemp,"%s","6.  ");	
    Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
	
		sprintf(DisTemp,"软件版本");
		Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
		
	  sprintf(DisTemp," V3.%02X.",SoftVersion);
    Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
		
  	Button1Value =  0x0002 ;
		
		Button2Value =  0x0001 ;
		
		HighLightItem	= 1 ;

    MenuWindowsDis(WindowID);	
	
}

void MenuWindows_Function(void)
{
	
	  u8   CurrentSelIndex = 1 ;
	
	  u16 key = 0xFF ;
	
	  MenuWindows_FunctionMenu();
	
	  KeyActiveStru.KeyOperate = 0xFF ;	
	
	  KeyActiveStru.KeyType = 0xFF ;

	  while(1)
		{		
			
		        HrnpSlaveRcvAndOperateCmdFromMaster() ;  //接收处理函数
						
			    	if(KeyActiveStru.KeyOperate!=0xFF)	
						{				
							
							     if(KeyActiveStru.KeyOperate==0x02)
									 {				    
																key = CovertKeyValue(KeyActiveStru.KeyType) ;
																	
																AudioSpkOpen();	
													 
																printf("\r\n Key_Vale_%02X Release ! \r\n",key) ;

														    if(key==KEY_XIAOHAO_VALUE||key==KEY_LEFT_VALUE)               //返回
																{
																	     wtn_play_voice_one(50); 
																			 return ;
																}
																
										            wtn_play_voice_one(28);
																
																if(key==KEY_UP_VALUE)  
																{
																	 
																		if(CurrentSelIndex>1)
																				 CurrentSelIndex-- ;
																		else
																				 CurrentSelIndex = 6 ;
																		
																		 HighLightItem = CurrentSelIndex ;
																		
																		 MenuWindowsDis(WindowID);		
																}
																else if(key==KEY_DOWN_VALUE)  
																{
																  	
																		if(CurrentSelIndex<6)
																				 CurrentSelIndex++ ;
																		else
																				 CurrentSelIndex =1 ;
																
																			HighLightItem = CurrentSelIndex ;
																		
																			MenuWindowsDis(WindowID);		
																}
																
																else if(key==KEY_QUEREN_VALUE||key==KEY_RIGHT_VALUE)               //选择
																{
																				key = CurrentSelIndex ;  
																}
																
	
															 if(key>=KEY_SHUZI1_VALUE&&key<=KEY_SHUZI6_VALUE)
															 {
																	
																				if(key==KEY_SHUZI1_VALUE)
																				{
																							MenuWindows_SetIDPara();
																				}
																				else if(key==KEY_SHUZI2_VALUE)
																				{
																							MenuWindows_QuryMoudle(); 
																				} 
																				else if(key==KEY_SHUZI3_VALUE)
																				{
																							MenuWindows_QuryGSMR();
																				}
																				else if(key==KEY_SHUZI4_VALUE)
																				{
																							MenuWindows_ParaRead();
																				} 																	
																				else if(key==KEY_SHUZI5_VALUE)
																				{
																							 MenuWindows_ParaSet();
																				}
																				else if(key==KEY_SHUZI6_VALUE)
																				{
																					     if(MenuWindows_PassWord()==2)
																							 {
																							        MenuWindows_DeviceSet();
																							 }
																				}
                                        CurrentSelIndex = 1 ;
																				MenuWindows_FunctionMenu();
																}
		
																else
																{
				
																}

																KeyActiveStru.KeyType = 0xFF ;											 
						        }
									 
									 KeyActiveStru.KeyOperate = 0xFF ; 	 
            }    
						
						if(T1S_Flag==1)
						{  
									RUN_LED__TURN ;
									IWDG_ReloadCounter();    //喂狗
						}
		}
}


//MenuWindows_MoudleStatus(FengYaZhi,ReceTime);
bool MenuWindows_MoudleStatus(u16 FengYaValue,u32 ReceTime)
{
	          u8  Temp[4] = {0,0,0,0};
	
	          Temp[0] = (ReceTime>>24)&0xFF ;
						Temp[1] = (ReceTime>>16)&0xFF ;
						Temp[2] = (ReceTime>>8)&0xFF ;
						Temp[3] = (ReceTime)&0xFF ;
					

						
						printf("\r\n MenuWindows_MoudleStatus---%4x---%8x  \r\n",FengYaValue,ReceTime);
						Item2_ItemLen = 0 ;
						sprintf(DisTemp,"%s","操作");						
						
						Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;	
		         
	          Item2_Text[4] = 0x1A ;
	          Item2_Text[5] = 0xFF ;
	          Item2_ItemLen = 3 ;
						
	          sprintf(DisTemp,"%s","状态应答  ");						
						
						Item2_ItemLen += HanZiCovertUniCode(DisTemp,Item2_Text+Item2_ItemLen*2) ;
						
		         
	          Item3_Text[4] = 0x1A ;
	          Item3_Text[5] = 0xFF ;
	
	          Item3_ItemLen = 0 ;						
									
	          sprintf(DisTemp,"%s","模块");						
						
						Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
		         
	          Item3_Text[4] = 0x1A ;
	          Item3_Text[5] = 0xFF ;
	
	          Item3_ItemLen = 3 ;
						

						Item4_ItemLen = 0 ;
	          sprintf(DisTemp,"%s","状态");						
						
						Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
		         
	          Item4_Text[4] = 0x1A ;
	          Item4_Text[5] = 0xFF ;
						
						Item4_ItemLen = 3 ;
						 
						
						if((FengYaValue&0x0C)==0x00)
						{	
												sprintf(DisTemp,"%s","已配");
												Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
									
												if((FengYaValue&0x03)==0)
												{

														    sprintf(DisTemp,"%s","  ");						
						                    Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
													
																sprintf(DisTemp,"%s","正常");
																Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2);	
													
													      if(Temp[1]==0x01)
																{											     
															    	sprintf(DisTemp,"%s","已注册");
																    Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;   
																}
																else
																{
																    sprintf(DisTemp,"%s","未注册");
																    Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ; 
																}
																
																sprintf(DisTemp,"    %d",Temp[0]);
																Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;
													
												}
												else if((FengYaValue&0x03)==1)
												{
																sprintf(DisTemp,"%s","故障");
																Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
												}		
						}	
						else
						{	
							      Item3_ItemLen = 3 ;  
										sprintf(DisTemp,"%s","未配置");
										Item3_ItemLen += HanZiCovertUniCode(DisTemp,Item3_Text+Item3_ItemLen*2) ;	
	
										Item4_ItemLen = 3 ;  
										sprintf(DisTemp,"%s","无效");
										Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
							
						}
						
						Item5_ItemLen = 0 ;
	          sprintf(DisTemp,"%s","卫星");						
						
						Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
		         
	          Item5_Text[4] = 0x1A ;
	          Item5_Text[5] = 0xFF ;
						
						Item5_ItemLen = 3 ;
						
						if((FengYaValue&0xC0)==0x00)
						{	
										sprintf(DisTemp,"%s","已配");
										Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
							
										sprintf(DisTemp,"%s","  ");						
						        Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
							
										if((FengYaValue&0x30)==0)
										{
														sprintf(DisTemp,"%s","正常");
														Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
										}
										else if((FengYaValue&0x30)==1)
										{
														sprintf(DisTemp,"%s","故障");
														Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
										}
						}	
						else if((FengYaValue&0xC0)==0x01)
						{	
										sprintf(DisTemp,"%s","未配置");
										Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;													
						}

						Item6_ItemLen = 0 ;
						sprintf(DisTemp,"HDOP:%02d STA:%d",Temp[2],Temp[3]);
						Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
						
						MenuWindowsDis(0xFFFFFFFF);		
}

bool MenuWindows_Opration(uint8_t SendExternCommandIndex,uint8_t SendCommandOprationIndex)
{
					    u8  year;
							u8   mon;
							u8   mday;
							u8   hour;
							u8   min;
							u8   sec ;
	            u8   tempip[4] ;
	            
	            Item4_ItemLen = 0 ;
	
	            sprintf(DisTemp,"%s","项目");
	
							Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
		         
	            Item4_Text[4] = 0x1A ;
	            Item4_Text[5] = 0xFF ;
	
	            Item4_ItemLen = 3 ;
	
							if(SendExternCommandIndex==0xD9||SendExternCommandIndex==0xDA||SendExternCommandIndex==0xDB)
							{		
//											if(SendExternCommandIndex==0xD9)
//											{
//															sprintf(DisTemp,"%s","查询");
//													    Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
//											}
//											else if(SendExternCommandIndex==0xDA)
//											{
//															sprintf(DisTemp,"%s","设置"); 
//													    Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;	
//											}
								
												if(SendCommandOprationIndex==0)
												{
															 sprintf(DisTemp,"%s","主机时间");  
															 Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;																 
												}
												else if(SendCommandOprationIndex==1)
												{
															 sprintf(DisTemp,"%s","APN");  
															 Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;														
												}
												else if(SendCommandOprationIndex==2)
												{
															 sprintf(DisTemp,"%s","主用");   
															 Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2);	
															 sprintf(DisTemp,"%s","DNS");
															 Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;																		
												}
												else if(SendCommandOprationIndex==3)
												{
															 sprintf(DisTemp,"%s","备用");   
															 Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2);	
													
															 sprintf(DisTemp,"%s","DNS");
															 Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;																		
												}
												else if(SendCommandOprationIndex==4)
												{
															 sprintf(DisTemp,"%s","RMS IP");
															 Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2) ;										
												}
												else if(SendCommandOprationIndex==5)
												{
															 sprintf(DisTemp,"%s","公网端口");
															 Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2);
																												
												}
												else if(SendCommandOprationIndex==6)
												{
															 sprintf(DisTemp,"%s","公网");
															 Item4_ItemLen += HanZiCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2);	
													
															 sprintf(DisTemp,"%s","IP");
															 Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text+Item4_ItemLen*2);													
											}	
			
						}
						if(SendExternCommandIndex==0xDB)
						{	
							         if(SendCommandOprationIndex==0)
											 {
												 				year =  (ReceTime>>26)&0x3F;
																mon =   (ReceTime>>22)&0x0F; 
																mday =  (ReceTime>>17)&0x1F;
																hour = (ReceTime>>12)&0x1F;
																min=  (ReceTime>>6)&0x3F;
																sec=  ReceTime&0x3F;
												 
												 
																sprintf(DisTemp,"%s","日期");
					
																Item5_ItemLen = 0 ;  
																Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
															 
																Item5_Text[4] = 0x1A;
																Item5_Text[5] = 0xFF;
											
																Item5_ItemLen = 3 ;
												 
												        sprintf(DisTemp,"%02d-%02d-%02d",year,mon,mday);
															  Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2);			

												 				sprintf(DisTemp,"%s","时间");
					
																Item6_ItemLen = 0 ;  
																Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
															 
																Item6_Text[4] = 0x1A;
																Item6_Text[5] = 0xFF;
											
																Item6_ItemLen = 3 ; 
												 
												        sprintf(DisTemp,"%02d:%02d:%02d",hour,min,sec);
																
																Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2);			 
											 }
											 else if(SendCommandOprationIndex==1)
											 {
																sprintf(DisTemp,"%s","编号");
					
																Item5_ItemLen = 0 ;  
																Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
															 
																Item5_Text[4] = 0x1A;
																Item5_Text[5] = 0xFF;
											
																Item5_ItemLen = 3 ;
												 
												        tempip[0] =  (ReceTime>>24)&0xFF;
												        sprintf(DisTemp,"  %02d  ",tempip[0]);
												 
															  Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2);			

											 }
											 else if(SendCommandOprationIndex==5)
											 {
																tempip[0] =  (ReceTime>>8)&0xFF;
																tempip[1] =  (ReceTime)&0xFF;
				
																sprintf(DisTemp,"%s","端口");
					
																Item5_ItemLen = 0 ;  
																Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
															 
																Item5_Text[4] = 0x1A;
																Item5_Text[5] = 0xFF;
											
																Item5_ItemLen = 3 ;
												 
												        sprintf(DisTemp,"  %02d%02d ",tempip[0],tempip[1]);
															  Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2);			

											 }	
							         else if(SendCommandOprationIndex>=2&&SendCommandOprationIndex<=4||SendCommandOprationIndex==6)
											 {
												 				tempip[0] =  (ReceTime>>24)&0xFF;
																tempip[1] =  (ReceTime>>16)&0xFF; 
																tempip[2] =  (ReceTime>>8)&0xFF;
																tempip[3] =  (ReceTime)&0xFF;
				
																sprintf(DisTemp,"%s","地址");
					
																Item5_ItemLen = 0 ;  
																Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
															 
																Item5_Text[4] = 0x1A;
																Item5_Text[5] = 0xFF;
											
																Item5_ItemLen = 3 ;
												 
												        sprintf(DisTemp,"%d:%d:%d:%d",tempip[0],tempip[1],tempip[2],tempip[3]);
															  Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2);			

											 }											 
						}

							MenuWindowsDis(0xFFFFFFFF);		
							
}
void MenuWindows_Clear(void)
{	
	  sprintf(DisTemp,"%s","               ") ;
	
	  TitleTextLen = 0 ;
	  Item1_ItemLen = 0 ;
	  Item2_ItemLen = 0 ;
	  Item3_ItemLen = 0 ;
	  Item4_ItemLen = 0 ;
	  Item5_ItemLen = 0 ;
	  Item6_ItemLen = 0 ;
	
		TitleTextLen +=  AscllCovertUniCode(DisTemp,TitleText)   ;
		Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text) ;
		Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text) ;
		Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text) ;
		Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text) ;		
		Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text) ;
		Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text) ;
				
	  MenuWindowsDis(WindowID);		
	
}
void MenuWindows_Refresh(void)
{	
	  u8  i = 0 ;
	
	  sprintf(DisTemp,"%s","             ") ;
	
	  TitleTextLen = 0 ;
	  Item1_ItemLen = 0 ;
	  Item2_ItemLen = 0 ;
	  Item3_ItemLen = 0 ;
	  Item4_ItemLen = 0 ;
	  Item5_ItemLen = 0 ;
	  Item6_ItemLen = 0 ;
	  
	  for(i=0;i<30;i++)
		{
				Item1_Text[i] = 0x00;
				Item2_Text[i] = 0x00;
				Item3_Text[i] = 0x00;
				Item4_Text[i] = 0x00;
				Item5_Text[i] = 0x00;
				Item6_Text[i] = 0x00;
		}
	
	  Item1_Text[0] = '\0';
	  Item2_Text[0] = '\0';
	  Item3_Text[0] = '\0';
	  Item4_Text[0] = '\0';
	  Item5_Text[0] = '\0';
	  Item6_Text[0] = '\0';

//		TitleTextLen +=  AscllCovertUniCode(DisTemp,TitleText)  ;
//		Item1_ItemLen += AscllCovertUniCode(DisTemp,Item1_Text) ;
//		Item2_ItemLen += AscllCovertUniCode(DisTemp,Item2_Text) ;
//		Item3_ItemLen += AscllCovertUniCode(DisTemp,Item3_Text) ;
//		Item4_ItemLen += AscllCovertUniCode(DisTemp,Item4_Text) ;		
//		Item5_ItemLen += AscllCovertUniCode(DisTemp,Item5_Text) ;
//		Item6_ItemLen += AscllCovertUniCode(DisTemp,Item6_Text) ;
					
}
bool MenuWindows_Start(uint32_t WindowID)
{
			 bool  result = FALSE ;
	     u8   i = 0  ;
	
			if(StartWaitQueRenFlag==WaitQueRen_SetJno||StartWaitQueRenFlag==WaitQueRen_SetIdnum||StartWaitQueRenFlag==WaitQueRen_XiaoHao)
			{
							return 0 ;  
			}
			else if(WorkMode_LwOrCall==2)
			{
							return 0 ;  
			}

	
		printf("\r\n Display MenuWindows_Start!\r\n");

   	MenuWindows_Refresh();

		TitleText[0] = 0x91  ;
		TitleText[1] = 0x98  ;	
		TitleText[2] = 0xB9  ;
		TitleText[3] = 0x70  ;		 
		TitleText[4] = 0x1A  ;
		TitleText[5] = 0xFF  ;
		
		Item1_Text[0] = 0x3A ;
		Item1_Text[1] = 0x67 ;	
		Item1_Text[2] = 0x66 ;
		Item1_Text[3] = 0x8F ;		 
		Item1_Text[4] = 0x1A ;
		Item1_Text[5] = 0xFF ;

		Item2_Text[0] = 0x17 ;
		Item2_Text[1] = 0x52 ;	
		Item2_Text[2] = 0x3E ;
		Item2_Text[3] = 0x5C ;		 
		Item2_Text[4] = 0x1A ;
		Item2_Text[5] = 0xFF ;
	 
		Item3_Text[0] = 0xCD ;
		Item3_Text[1] = 0x64 ;	
		Item3_Text[2] = 0x5C ;
		Item3_Text[3] = 0x4F ;		 
		Item3_Text[4] = 0x1A ;
		Item3_Text[5] = 0xFF ;

		Item4_Text[0] = 0xCE ;
		Item4_Text[1] = 0x98 ;	
		Item4_Text[2] = 0x8B ;
		Item4_Text[3] = 0x53 ;		 
		Item4_Text[4] = 0x1A ;
		Item4_Text[5] = 0xFF ;

		Item5_ItemLen = 0 ; 	
		sprintf(DisTemp,"%s","时间");
		Item5_ItemLen += HanZiCovertUniCode(DisTemp,Item5_Text+Item5_ItemLen*2) ;	
	 
		Item5_Text[4] = 0x1A;
		Item5_Text[5] = 0xFF;

		Item5_ItemLen = 3 ;

		sprintf(DisTemp,"%s","场强");
		Item6_ItemLen += HanZiCovertUniCode(DisTemp,Item6_Text+Item6_ItemLen*2) ;	
		
		Item6_Text[4] = 0x1A;
		Item6_Text[5] = 0xFF;
		
		Item6_ItemLen = 3 ;
		
		TitleTextLen  = 3 ;
		Item1_ItemLen = 3 ;
		Item2_ItemLen = 3 ;
		Item3_ItemLen = 3 ;
		Item4_ItemLen = 3 ;		
		Item5_ItemLen = 3 ;
    Item6_ItemLen = 3 ;
		
		if(WorkXinDaoNum==1)
		{
						CurrentWorkMode = WorkMode_ShuZi ;
						sprintf(DisTemp,"%s","404.025");
		}
    else if(WorkXinDaoNum==2)
		{
						CurrentWorkMode = WorkMode_ShuZi ;
						sprintf(DisTemp,"%s","414.025");
		}  	
		else if(WorkXinDaoNum==11)
		{
					 CurrentWorkMode = WorkMode_ShuZi ;
					 sprintf(DisTemp,"%s","408.550");
		}
		else if(WorkXinDaoNum==12)
		{
					 CurrentWorkMode = WorkMode_ShuZi ;
					 sprintf(DisTemp,"%s","418.550");
		}			  
		
    TitleTextLen += AscllCovertUniCode(DisTemp,TitleText+TitleTextLen*2) ;
		
		if(RadioJianTingMode==1)
		{
						sprintf(DisTemp,"监听");
			
						TitleTextLen += HanZiCovertUniCode(DisTemp,TitleText+20) ;
		}
		
		ShowDispJNO(JNO) ;
    Item1_ItemLen = AscllCovertUniCode(DisTemp,Item1_Text+6) + 3 ;
		
    if(IDNUM>=0xA00000)
		{
			    sprintf(DisTemp,"%s","      ");
		}
		else
		{
		      sprintf(DisTemp,"%06X",IDNUM);
			    RadioJianTingMode = 0 ;
		}

		Item2_ItemLen = AscllCovertUniCode(DisTemp,Item2_Text+6) + 3 ;
		
		for(i=0;i<3;i++)
		{
	       result = MenuWindowsDis(WindowID);
			   if(result==TRUE)
				 {
				     break ;
				 }
		}			
		
}
/***************************************************************************
Function name   :MenuWindowsDis
Description     :open audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool MenuWindowsDis(uint32_t value)
{
    SendResult_t CheckResult = SEND_RESULT_SUCCESSFUL;
    FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
    bool result = FALSE;	
	  uint16_t  index = 0 ; 
	  uint8_t  DisArry[260] ;
		uint8_t  i= 0 ;
	  uint8_t  timeout = 1 ;
	
    //WindowID
    DisArry[index++] =  WindowID&0xFF ;  
    DisArry[index++] = (WindowID>>8)&0xFF ;
    DisArry[index++] = (WindowID>>16)&0xFF ;
    DisArry[index++] = (WindowID>>24)&0xFF ;

	  /* 越界保护：本函数固定写入 228 字节，标题再占 2*TitleTextLen 字节；
	     TitleText[30] 最多容纳 15 个字，上限即 228+30=258，故 DisArry 取 260。
	     原先 250 在标题 14~15 字时(256~258)会越过本函数的返回地址。 */
	  if(TitleTextLen > (sizeof(TitleText)/2))
	  {
	  	TitleTextLen = sizeof(TitleText)/2 ;
	  }
	  //标题长度
    DisArry[index++] =  TitleTextLen&0xFF ;  
    DisArry[index++] = (TitleTextLen>>8)&0xFF ;				
		//标题内容
		for(i=0;i<TitleTextLen;i++)
		{
         DisArry[index++] = TitleText[2*i] ;  
         DisArry[index++] = TitleText[2*i+1] ;
    }
		
		DisArry[index++] =  Button1Value&0xFF ;  
    DisArry[index++] = (Button1Value>>8)&0xFF ;		
		DisArry[index++] =  Button2Value&0xFF ;  
    DisArry[index++] = (Button2Value>>8)&0xFF ;	
		
		DisArry[index++] = HighLightItem ;
		
		DisArry[index++] = ItemNum ;
		
		DisArry[index++] = ItemIndex1 ;	
		DisArry[index++] = Item1_Icon ;
		DisArry[index++] = Item1_LineFeed ;
		DisArry[index++] = Item1_Reserved ;
		
		Item1_ItemLen    = sizeof(Item1_Text)/2 ;
		
		DisArry[index++] = Item1_ItemLen&0xFF ; 
		DisArry[index++] = (Item1_ItemLen>>8)&0xFF ;
		
		for(i=0;i<Item1_ItemLen;i++)
		{
         DisArry[index++] = Item1_Text[2*i] ;  
         DisArry[index++] = Item1_Text[2*i+1];
    }
		
		DisArry[index++] = ItemIndex2 ;	
		DisArry[index++] = Item2_Icon ;
		DisArry[index++] = Item2_LineFeed ;
		DisArry[index++] = Item2_Reserved ;
		Item2_ItemLen    = sizeof(Item2_Text)/2 ;
		DisArry[index++] = Item2_ItemLen&0xFF ; 
		DisArry[index++] = (Item2_ItemLen>>8)&0xFF ;
		
		for(i=0;i<Item2_ItemLen;i++)
		{
         DisArry[index++] = Item2_Text[2*i] ;  
         DisArry[index++] = Item2_Text[2*i+1];
    }
		
		DisArry[index++] = ItemIndex3 ;	
		DisArry[index++] = Item3_Icon ;
		DisArry[index++] = Item3_LineFeed ;
		DisArry[index++] = Item3_Reserved ;
		Item3_ItemLen    = sizeof(Item3_Text)/2 ;
		DisArry[index++] = Item3_ItemLen&0xFF ; 
		DisArry[index++] = (Item3_ItemLen>>8)&0xFF ;
		
		for(i=0;i<Item3_ItemLen;i++)
		{
         DisArry[index++] = Item3_Text[2*i] ;  
         DisArry[index++] = Item3_Text[2*i+1];
    }
		
		DisArry[index++] = ItemIndex4 ;	
		DisArry[index++] = Item4_Icon ;
		DisArry[index++] = Item4_LineFeed ;
		DisArry[index++] = Item4_Reserved ;
		
		Item4_ItemLen    = sizeof(Item4_Text)/2 ;
		DisArry[index++] = Item4_ItemLen&0xFF ; 
		DisArry[index++] = (Item4_ItemLen>>8)&0xFF ;
		
		for(i=0;i<Item4_ItemLen;i++)
		{
         DisArry[index++] = Item4_Text[2*i] ;  
         DisArry[index++] = Item4_Text[2*i+1];
    }
		
		DisArry[index++] = ItemIndex5 ;	
		DisArry[index++] = Item5_Icon ;
		DisArry[index++] = Item5_LineFeed ;
		DisArry[index++] = Item5_Reserved ;
			
	 Item5_ItemLen    = sizeof(Item5_Text)/2 ;
	 DisArry[index++] = Item5_ItemLen&0xFF ; 
	 DisArry[index++] = (Item5_ItemLen>>8)&0xFF ;
		
	 for(i=0;i<Item5_ItemLen;i++)
	 {
         DisArry[index++] = Item5_Text[2*i] ;  
         DisArry[index++] = Item5_Text[2*i+1];
   }
	 
	 
	 	DisArry[index++] = ItemIndex6 ;	
		DisArry[index++] = Item6_Icon ;
		DisArry[index++] = Item6_LineFeed ;
		DisArry[index++] = Item6_Reserved ;
			
	  Item6_ItemLen    = sizeof(Item6_Text)/2 ;
	  DisArry[index++] = Item6_ItemLen&0xFF ; 
	  DisArry[index++] = (Item6_ItemLen>>8)&0xFF ;
		
	  for(i=0;i<Item6_ItemLen;i++)
	  {
         DisArry[index++] = Item6_Text[2*i] ;  
         DisArry[index++] = Item6_Text[2*i+1];
    }
		while(1)
		{
					CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, Window_DisPlay_Menu, Window_DisPlay_Menu_REPLY, DisArry, index, HrnpReplyMsg);
					
					if(SEND_RESULT_FAILED == CheckResult)
					{
							  SystemStatus = STATUS_SYSTEM_FREE;
						    printf("\r\nMenuWindowsOpen Index--%d Fail\r\n",WindowID);
						    MenuWindows_DisOk = 0 ;
						    return result;		
					}
					else if(SEND_RESULT_SUCCESSFUL == CheckResult)
					{
								 SystemStatus = STATUS_SYSTEM_FREE;
								 WindowID = HrnpReplyMsg->RcvFrame[21]<<24|HrnpReplyMsg->RcvFrame[20]<<16|HrnpReplyMsg->RcvFrame[19]<<8|HrnpReplyMsg->RcvFrame[18] ;
								 printf("\r\nMenuWindowsOpen Index--%d Ok\r\n",WindowID);
								 result = TRUE;	
						     MenuWindows_DisOk = 1 ;
								 return result;						
					}
//					timeout++ ;
//					if(timeout>2)
//					{
//					    return result;		
//					}
	  }
}


void ShowDispJNO(u32 tempjno)
{
	     if(JNO==0x0)
			 {
			      sprintf(DisTemp,"%s","            ");	
				    JNO = 0xAAAAAAAA ;
			 }
			 else if(CheckJNO(tempjno)==1)	 
			 {
							sprintf(DisTemp,"%08X   ",tempjno);	
			 }
			 else 
			 {
							sprintf(DisTemp,"%s","            ");	
			 }
}
/***************************************************************************
Function name   :PlayAudioSpkSwitchProcess
Description     :respone the key msg to control the spk switch when play audio
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
void PlayAudioSpkSwitchProcess(void)
{
    if(gSpkDisableReq)
    {
          AudioSpkClose();
    }
 
    else if(gSpkEnableReq)
    {
          AudioSpkOpen();
    }
    else
    {

    }
		
}

/***************************************************************************
Function name   :ChangeRadioVolume
Description     :change radio speaker volume when play back record voice
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
void ChangeRadioVolume(void)
{
	
//			 SendResult_t CheckResult    = SEND_RESULT_SUCCESSFUL;
//	
//			 FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
//	
//			 uint8_t AudioPathGainCfg[]  = {0x00,0x00,0x00,0x00};

////     AudioPathGainCfg[1] = 0x00;      //  0x00: to configure the gain   0x01: to check the gain

//			 AudioPathGainCfg[2] = (uint8_t) RadioVolumeValue ;

//			 AudioPathGainCfg[3] = (uint8_t)(RadioVolumeValue>>8);

//			 while(1)
//			 {
//				 
//									CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, AUDIO_PATH_GAIN_CFG, AUDIO_PATH_GAIN_CFG_REPLY, AudioPathGainCfg, sizeof(AudioPathGainCfg), HrnpReplyMsg);
//				 
//									if(SEND_RESULT_FAILED == CheckResult)
//									{
//											SystemStatus = STATUS_SYSTEM_FREE;
//										  return ;
//									}
//									else if(SEND_RESULT_SUCCESSFUL == CheckResult)
//									{
//											gVolumeChangeTimeOut = 0;
//											SystemStatus = STATUS_SYSTEM_FREE;
//										  return ;
//									}
//									else
//									{
//													
//									} 
//									
//							
//			 }
}
/***************************************************************************
Function name   :ChangeRadioVolume
Description     :change radio speaker volume when play back record voice
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool WindowBackLightCon(uint8_t setvalue)
{
			 SendResult_t CheckResult    = SEND_RESULT_SUCCESSFUL;
	
			 FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
	     bool Result = FALSE;
	
			 uint8_t WindowBackLightValue[]  = {0x00};

			 WindowBackLightValue[0] = setvalue;         //0x00: Disable      0x01: Enable
 
			 while(1)
			 {			 
									CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, Window_BackLight_Con, Window_BackLight_Con_REPLY, WindowBackLightValue, sizeof(WindowBackLightValue), HrnpReplyMsg);
									if(SEND_RESULT_FAILED == CheckResult)
									{
													 SystemStatus = STATUS_SYSTEM_FREE;
													 BackLinghtStatus= 0;
													 BackLinghtCount = 0 ;
													 return Result;	
									}
									else if(SEND_RESULT_SUCCESSFUL == CheckResult)
									{
													SystemStatus = STATUS_SYSTEM_FREE ;
										
													if(setvalue==1)
													{
																 BackLinghtStatus = 1 ;
													       BackLinghtCount = 30 ;
																 printf("\r\n Open BackLight Command Ok!\r\n");
													}
													else
													{
														     BackLinghtStatus= 0;
														     BackLinghtCount = 0 ;
																 printf("\r\n Close BackLightCon Command  Ok!\r\n");
													}
													Result = TRUE;
													return Result;
									}		
			 }
}


/***************************************************************************
Function name   :ChangeRadioVolume
Description     :change radio speaker volume when play back record voice
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool RadioSendCommandFrame(uint32_t CommandOpCode,uint32_t CommandOpCodeREPLY,uint8_t *arrdata)
{
	
			 SendResult_t CheckResult    = SEND_RESULT_SUCCESSFUL;
	
			 FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
	     bool Result = FALSE;
	
			 while(1)
			 {
									CheckResult = HrnpSlaveSendRqToMaster(RCPMODE, CommandOpCode, CommandOpCodeREPLY, arrdata, sizeof(arrdata), HrnpReplyMsg);
				 
									if(SEND_RESULT_FAILED == CheckResult)
									{
											 SystemStatus = STATUS_SYSTEM_FREE;
										   printf("\r\n Send %04X Command Is Fail!!!\r\n",CommandOpCode);
										   return Result;
									}
									else if(SEND_RESULT_SUCCESSFUL == CheckResult)
									{
											SystemStatus = STATUS_SYSTEM_FREE;
										  if(SUCCESSFUL == HrnpReplyMsg->RcvFrame[17])
											{
													 Result =  TRUE;
												   printf("\r\n Send %04X Command Is Ok.\r\n",CommandOpCode);
											}
										  Result = TRUE;
										  return Result;
									}
									else
									{
										
									}     
							
			 }
}

u16 GetCrc16CheckResult(u8 *ptr,u8 len)        
{     
				u8 i; 
	
				u16  crc = 0 ;
	
				while(len--)    
				{        
								  i=0x80;     
									while(i!=0)    
									{       
									if(((crc&0x8000)!= 0) && ((*ptr)&i)!= 0)      
									{            
									crc <<= 1;      
									}       
									else if(((crc&0x8000)!= 0) && ((*ptr)&i) == 0)      
									{            
									crc <<= 1;       
									crc ^= 0x1021;      
									}             
									else if(((crc&0x8000) == 0) && ((*ptr)&i)!= 0)            
									{            
									crc <<= 1;       
									crc ^= 0x1021;            
									}          
									else if(((crc&0x8000) == 0) && ((*ptr)&i) == 0)         
									{               
									crc <<= 1;         
									}      
									i >>= 1;   
									}     
									ptr++;    

				}     
				return(crc);   
}

extern u8 MAX_SEND_TIMES  ;  
extern u16 TIME_FOR_ACK ;  
/***************************************************************************
Function name   :AudioSpkClose
Description     :close audio speaker
Input           :none
Output          :none
Return          :none
Others          :none
***************************************************************************/ 
bool RadioSendCsbkFunction(uint8_t *csbkdataone,uint8_t *csbkdatatwo)
{
			SendResult_t CheckResult    = SEND_RESULT_SUCCESSFUL;
	
			 FrameBuf_t *HrnpReplyMsg = &g_HrnpFrameScratch0;
	
	     bool Result = FALSE;
	
	     uint8_t i=1,j=0,k=0,m=0,n=0;
	
	     u8     CsbkCrcData[14];
	
	     u16    CrcData = 0;
	
	     uint8_t sendcsbkdata[181] = {0x01};    //【0】时隙为0x01

			 i = 1 ;
			 
			 for(j=3; j>0; j--)
			 {
									csbkdataone[3] |= j<<3 ;
				 
									for(n=0;n<7;n++)
									{
												CsbkCrcData[n]   =  csbkdataone[n+3] ;
												CsbkCrcData[7+n] =  csbkdatatwo[n+3] ;
									}
							
									CrcData = GetCrc16CheckResult(CsbkCrcData,14);
									
									csbkdataone[2] = CrcData>>8&0xFF ;
									
									csbkdatatwo[2] = CrcData&0xFF ;
									
									for(k=3;k>0;k--)
									{
												for(m=0; m<10; m++)
												{				
														sendcsbkdata[i++] = csbkdataone[m] ;	
												}
												for(m=0; m<10; m++)
												{				
														sendcsbkdata[i++] = csbkdatatwo[m] ;					
												} 		
									}
			}
			IWDG_ReloadCounter() ;                         //喂狗
			 MAX_SEND_TIMES = 1 ;
			 TIME_FOR_ACK  =  1200 ;
			 while(1)
			 {
									CheckResult = HrnpSlaveSendRqToMaster(TMPMODE, Radio_SEND_CSBKFRAME, Radio_SEND_CSBKFRAME_REPLY, sendcsbkdata, sizeof(sendcsbkdata), HrnpReplyMsg);
				 
									if(SEND_RESULT_FAILED == CheckResult)
									{
													SystemStatus = STATUS_SYSTEM_FREE;
													printf("\r\n Send %04X Command Is Fail!!!\r\n",0x0C41);
										      TIME_FOR_ACK = 500  ;
													MAX_SEND_TIMES = 2;
													return Result ;
									}
									else if(SEND_RESULT_SUCCESSFUL == CheckResult)
									{
													SystemStatus = STATUS_SYSTEM_FREE ;
													if(SUCCESSFUL == HrnpReplyMsg->RcvFrame[17])
													{
															 Result =  TRUE ;
															 printf("\r\n Send %04X Command Is Ok.\r\n",0x0C41);
													}
													Result = TRUE ;
													TIME_FOR_ACK = 500  ;
													MAX_SEND_TIMES = 2 ;
													return Result ;
									} 
									if(T1S_Flag==1)
									{  
													RUN_LED__TURN ;	 
													IWDG_ReloadCounter() ;                         //喂狗
													T1S_Flag = 0 ;
									}											
			 }
}

