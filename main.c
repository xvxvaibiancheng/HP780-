#include "stm32f10x.h"
#include "delay.h"
#include "bsp.h"
#include "apprecording.h"
#include <stdio.h>

#define		RUN_LED_ON()						GPIO_ResetBits(GPIOB , GPIO_Pin_13); 			//PA8 输出低电平，使其发光
#define		RUN_LED_OFF()					  GPIO_SetBits(GPIOB , GPIO_Pin_13); 			  //PA8 输出高电平，使其灭
#define   RUN_LED__TURN           GPIO_WriteBit(GPIOB, GPIO_Pin_13, (BitAction)(1-GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13)))


#define		AudioOut_Sel_FSK()			GPIO_ResetBits(GPIOA , GPIO_Pin_8); 			//语音输出信号选择为FSK
#define		AudioOut_Sel_SPK()			GPIO_SetBits(GPIOA , GPIO_Pin_8); 			  //PA8 输出高电平，使其灭

void LED_Pin_Init()
{
	
				GPIO_InitTypeDef  GPIO_InitStructure;
				
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	   //使能PB端口时钟
	
	      RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	      GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		     //推挽输出
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		     //IO口速度为50MHz
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;	             //AudioOut_Sel端口配置, 推挽输出
				GPIO_Init(GPIOA, &GPIO_InitStructure);

	      AudioOut_Sel_SPK();
				
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		     //推挽输出
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		     //IO口速度为50MHz
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 ;	             //LED---PB端口配置, 推挽输出
				GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	      GPIO_SetBits(GPIOB , GPIO_Pin_13) ; 	

}

u8 USART2_RX_BUF[100] = {0} , USART2_i = 0 , USART2_OVER_Time = 0;
u8 USART3_RX_BUF[10] = {0} , USART3_i = 0 , USART3_OVER_Time = 0;

//初始化TIM4控制引脚及配置其中断方式、优先级
void TIM4_Int_Init(u16 arr , u16 psc)
{
	
					TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
					NVIC_InitTypeDef NVIC_InitStructure;

					RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //使能定时器4时钟

					TIM_TimeBaseStructure.TIM_Period = arr; 		//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到2000为2ms
					TIM_TimeBaseStructure.TIM_Prescaler = psc; 	//设置用来作为TIMx时钟频率除数的预分频值  1Mhz的计数频率  
					TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 		 //设置时钟分割:TDTS = Tck_tim
					TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
					TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
				 

					TIM_ITConfig(TIM4 , TIM_IT_Update , ENABLE ); //使能指定的TIM4中断,允许更新中断
					
					NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
					NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
					NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
					NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 		//IRQ通道被使能
					NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

					TIM_Cmd(TIM4, ENABLE);  //使能TIM4外设,开始计数		

}
void BKP_Configuration(void)
{
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ê1?üPWRoíBKPíaéèê±?ó   
		  PWR_BackupAccessCmd(ENABLE);	//ê1?üoó±???′??÷·??ê 
}

void Usart1_init(unsigned long int bound)
{
			//GPIO端口设置
			GPIO_InitTypeDef GPIO_InitStructure;
			USART_InitTypeDef USART_InitStructure;
			NVIC_InitTypeDef NVIC_InitStructure;
			 
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
			
			//USART1_TX   GPIOA.9
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 					//PA.9
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
			GPIO_Init(GPIOA, &GPIO_InitStructure);							//初始化GPIOA.9
			 
			//USART1_RX	  GPIOA.10初始化
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;						//PA10
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
			GPIO_Init(GPIOA, &GPIO_InitStructure);								//初始化GPIOA.10  

			//Usart1 NVIC 配置
			NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;			//子优先级2
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQ通道使能
			NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
			
			 //USART 初始化设置

			USART_InitStructure.USART_BaudRate = bound;									//串口波特率
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
			USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一个停止位
			USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
			USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
			USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;									//收发模式

			USART_Init(USART1, &USART_InitStructure); 				//初始化串口1
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);		//开启串口接受中断
			USART_Cmd(USART1, ENABLE);                    		//使能串口1 
}
extern u8 DebugMode ;
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	        if(DebugMode==0)
					{
					    return 0;	
					}
					while(USART_GetFlagStatus(USART2 , USART_FLAG_TXE) != SET);	//等待发送寄存器是否为空
					USART_SendData(USART2, (u8)ch);					
					return ch;
}
void Usart2_init(unsigned long int bound)
{
					//GPIO端口设置
					GPIO_InitTypeDef  GPIO_InitStructure ;
					USART_InitTypeDef USART_InitStructure ;
					NVIC_InitTypeDef  NVIC_InitStructure ;
					 
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2时钟
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//使能GPIOA时钟
					
					//USART2_TX   GPIOA.2
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 					//PA.2
					GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
					GPIO_Init(GPIOA, &GPIO_InitStructure);							//初始化GPIOA.2
					 
					//USART2_RX	  GPIOA.3初始化
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;						//PA3
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
					GPIO_Init(GPIOA, &GPIO_InitStructure);								//初始化GPIOA.3

					//Usart1 NVIC 配置
					NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
					NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
					NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//子优先级2
					NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQ通道使能
					NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
					
					//USART 初始化设置
					USART_InitStructure.USART_BaudRate = bound;									//串口波特率
					USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
					USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一个停止位
					USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
					USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
					USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;									//收发模式

					USART_Init(USART2, &USART_InitStructure); 				//初始化串口1
					USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);		//开启串口接受中断
					USART_Cmd(USART2, ENABLE);   //使能串口1 
					
					USART2_i = 0 ;
}
/*从串口2发送一个字节*/
void USART2_PutChar(u8 ch)
{
		  USART_SendData(USART2, (unsigned char) ch);
		  while (!(USART2->SR & USART_FLAG_TXE));
}
void Usart3_init(unsigned long int bound)
{
					//GPIO端口设置
					GPIO_InitTypeDef GPIO_InitStructure;
					USART_InitTypeDef USART_InitStructure;
					NVIC_InitTypeDef NVIC_InitStructure;
					 
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART2时钟
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//使能GPIOA时钟
					
					//USART3_TX   GPIOB.10
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 					//PA.2
					GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
					GPIO_Init(GPIOB, &GPIO_InitStructure);							//初始化GPIOA.2
					 
					//USART3_RX	  GPIOB.11初始化
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;						//PA3
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
					GPIO_Init(GPIOB, &GPIO_InitStructure);								//初始化GPIOA.3

					//Usart1 NVIC 配置
					NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
					NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
					NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;			//子优先级2
					NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQ通道使能
					NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
					
					//USART 初始化设置
					USART_InitStructure.USART_BaudRate = bound;									//串口波特率
					USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
					USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一个停止位
					USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
					USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
					USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;									//收发模式

					USART_Init(USART3, &USART_InitStructure); 				//初始化串口1
					USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);		//开启串口接受中断
					USART_Cmd(USART3, ENABLE);   //使能串口1 
				
}
/*从串口3发送一个字节*/
void USART3_PutChar(u8 ch)
{
		  USART_SendData(USART3, (unsigned char) ch);
		  while (!(USART3->SR & USART_FLAG_TXE));
}

void USART1_SendDataBuf(u8* buf, u16 len)
{ 
				u16 t; 
			
				for(t = 0 ; t < len ; t++)
				{	
							while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);	//等待发送寄存器是否为空			
							USART_SendData(USART1, buf[t]);													   //向串口1发送数据	
							while(USART_GetFlagStatus(USART1 , USART_FLAG_TC)!= SET);		//等待发送结束
				}	
}
void USART2_SendDataBuf(u8* buf, u16 len)
{ 
				u16 t ;
	
				for(t = 0 ; t < len ; t++)
				{	
							while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET);	  //等待发送寄存器是否为空			
							USART_SendData(USART2, buf[t]);													    //向串口2发送数据	
							while(USART_GetFlagStatus(USART2 , USART_FLAG_TC)!= SET);		//等待发送结束
				}
	
}
void USART3_SendDataBuf(u8* buf, u16 len)
{ 
	
				u16 t ;
	
	      printf(" \r\n Send WTV Command!--%d \r\n", len);
	
				for(t = 0 ; t < len ; t++)
				{	
							while(USART_GetFlagStatus(USART3,USART_FLAG_TXE) != SET);	  //等待发送寄存器是否为空			
							USART_SendData(USART3, buf[t]);													    //向串口3发送数据	
							while(USART_GetFlagStatus(USART3 , USART_FLAG_TC)!= SET);		//等待发送结束	
					    printf(" %02X",buf[t]);
				}
				
				 printf(" \r\n Send WTV Command OK!\r\n");
				
	
}
void IWDG_Configuration(void)
{
				/* 写入0x5555,用于允许狗狗寄存器写入功能*/
				IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

				/* 狗狗时钟分频,40K/256=156HZ(6.4ms)*/
				IWDG_SetPrescaler(IWDG_Prescaler_256);

				/* 喂狗时间 2.5s/6.4MS=781 注意不能大于0xfff*/
				IWDG_SetReload(1000);

				/* 喂狗·*/
				IWDG_ReloadCounter();

				/* 使能喂狗*/
				IWDG_Enable();
}
////////////////////////////////////////////////////////////////////////////////
// ????ê??μí3RCC?￡?éμ?3?ê??ˉ￡??a2?·?±è???′?ó
// ?3D?1|?ü?￡?é￡¨è?USART￡?RTCμè￡?μ??à1?ê±?óê1?ü￡?±?·??ú?÷×?μ??￡?é3?ê??ˉoˉêy?D
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : ?μí3ê±?óéè??
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
	
		/* RCC system reset(for debug purpose) */
	
		RCC_DeInit();

		RCC_HSICmd(ENABLE);

		RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

	  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY)==RESET)
	  {
			
	  }
	  if(1)
	  {
	  		    RCC_HCLKConfig(RCC_SYSCLK_Div1);   
			      /* PCLK2 = HCLK */
	          RCC_PCLK2Config(RCC_HCLK_Div1); 
	          /* PCLK1 = HCLK/2 */
	          RCC_PCLK1Config(RCC_HCLK_Div2); 
	  }

    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_9);

      /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);


	  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
			
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
			
    }

}
int main()
{
	    bool  res ;
	
			delay_init() ;				       //调用精确延时初始化函数
	
	    RCC_Configuration() ;
	
	    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000) ;
	
			TIM4_Int_Init(1000,36-1) ;
	
			LED_Pin_Init() ;
	
			Usart1_init(115200) ;     // 调用串口1初始化函数----选件板通信口
	
			Usart2_init(256000) ;		  // 调用串口2初始化函数----调试日志输出
	
		  Usart3_init(9600) ;		    // 调用串口3初始化函数----语音控制
	
	    BKP_Configuration() ;     // 备份寄存器配置
	
	    IWDG_Configuration();
						
	    while(1)  				       //无限循环，等待串口2接收数据中断产生		
			{
				    EnterGobUserMode(); 
			}
}

