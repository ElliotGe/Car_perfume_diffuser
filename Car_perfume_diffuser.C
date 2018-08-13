
/**********************************************************
*Project: Car_perfume_diffuser.prj
* Device: MS82Fxx02
* Memory: Flash 2KX14b, EEPROM 128X8b, SRAM 128X8b
* Author: Elliot Ge
*Company: Rezeey Inc.
*Version: V1.0
*   Date: 2018-8-1

*FUNCTION: RG LED, Motor with PWM, Button with int
*IC Device:MS82F1402B
*Oscillator:Inside RC 4MHz 4T
*PIN:
*                 ---------------
*  VDD-----------|1(VDD)   (VSS)8|------------GND
*  LED_G---------|2(PA6)   (PA1)7|------------CLK
*  LED_R---------|3(PC3)   (PA3)6|------------DAT
*  Motor---------|4(PC2)   (PC1)5|------------Button
*                 ----------------
*                 MS82F0802B SOP8
**********************************************************/


/**********************************************************
 * include files
**********************************************************/
#include	"SYSCFG.h"
#include 	"MS82Fxx02.h"


/**********************************************************
 * define
**********************************************************/
#define _XTAL_FREQ		4000000

#define LEDG 			RA6
#define LEDR			RC3
#define BUTTON 			RC1


#define LEDGEAR1T1		800
#define LEDGEAR1T2		0
#define LEDGEAR2T1		300
#define LEDGEAR2T2		150
#define LEDGEAR3T1		250
#define LEDGEAR3T2		150
#define LEDINTERVAL		20000


#define SLEEPTIMERGEAR1	2*60*60*1000
#define SLEEPTIMERGEAR2
#define SLEEPTIMERGEAR3	




#define RunStatusEEAddr 0x05

#define SET_INDICATOR_LED do {\
								if (LowPower == 0)\
								{\
									LEDG = 1; LEDR =0;\
								}\
								else\
								{\
									LEDR = 1;LEDG = 0;\
								}\
							}while(0)

#define CLEAR_INDICATOR_LED do {\
								if (LowPower == 0)\
								{\
									LEDG = 0; LEDR =0;\
								}\
								else\
								{\
									LEDR = 0;LEDG = 0;\
								}\
							}while(0)
/**********************************************************
*Variable definition
**********************************************************/
unsigned int LEDTimer;
unsigned int LEDT;



struct LED_Def
{
	unsigned int SBinkNum		:5;
	unsigned int IBinkNum		:5;
	unsigned int SRGB			:3;
	unsigned int IRGG			:3;
}LEDStatus;

unsigned int ButtonTimer;

struct BUTTON_Def
{
	unsigned char ButtonCH		:1;
	unsigned char ButtonCur		:1;
	unsigned char ButtonCount	:2;
	unsigned char 		:4;
}ButtonStatus;


unsigned int TimerCount;


unsigned int SleepTimer;


union 
{
	struct RUNSTATUS_Def
	{
		unsigned char EPWMGear		:4;
		unsigned char OffTimerGear	:4;
	}RunStatus;
	unsigned char RunStatusChar;
} URunStatus;

unsigned char LowPower;

/**********************************************************
*Funtion name：interrupt ISR
*parameters：
*returned value：
**********************************************************/
void interrupt ISR(void)
{
	if(TMR1IE&&TMR1IF)
	{
		TMR1IF = 0;
		TMR1H = 0x00;
  		TMR1L = 0x00;
		TimerCount += 1;
	}

	if(INTE&&INTF)
	{
		INTF = 0;          //清标志位
		INTE = 0;          //关闭中断
		WEEKUP_SETTING();
	}
}




/**********************************************************
*Funtion name：device init
*parameters：
*returned value：
**********************************************************/
void DEVICE_INIT(void)
{
	OSCCON 	= 0B01010001;   //Bit7    >>>  LFMOD=0 >>> WDT振荡器频率=32KHz
                           //Bit6:4 >>> IRCF[2:0]=101 >>> 内部RC频率=4MHz
                           //Bit0   >>> SCS=1      >>> 系统时钟选择为内部振荡器
	INTCON 	= 0B00000000; //暂禁止所有中断
	PIE1 	= 0B00000000;
	PIR1 	= 0B00000000;
	PORTA 	= 0B00000000;
	TRISA 	= 0B11111111;
	WPUA 	= 0B11111111;
	PORTC 	= 0b00000000;
	TRISC 	= 0B11111111;
	WPUC 	= 0B00000000;
	OPTION 	= 0B11111111;
}



void LED_INIT(void)
{
	TRISA	&= 0B10111111;
	WPUA 	&= 0B10111111;
	TRISC 	&= 0B11110111;
	WPUC	&= 0B11110111;
	LEDStatus = {0,0,0,0};
	LEDT = LEDINTERVAL;
}

void BUTTON_INIT(void)
{
	TRISC 	|= 0B00000010;
	WPUC	|= 0B00000010;
	ButtonStatus = {0,0,0};
}


/**********************************************************
*Funtion name：timer1 init
*parameters：none
*returned value： none
T1CON
Bit		7		6		5		4		3		2		1		0
Name	T1GINV	TIMR1GE	T1CKPS1 T1CKPS0	-		T1SYNC	TMR1CS TMR1ON

period = (1/Fosc)*2T(or 4T)*T1CKPS*(65535-TMR1H:TMR1L)
(1/4MHZ)*4*8*65535= 524.28ms
**********************************************************/
void TIMER_INIT(void)
{
	TMR1H = 0x00;
    TMR1L = 0x00; 
	T1CON = 0B00110001;
}


/**********************************************************
*Funtion name：low power init
*parameters：none
*returned value： none
PCON
Bit		7		6		5		4		3		2		1		0
Name	-	LVDL[2:0]				  LVDEN	   LVDW		
**********************************************************/
void LOW_POWER_INIT(void)
{
	LVDL = 2;
	LVDEN = 1;	
}


/**********************************************************
*Funtion name：main
*parameters：
*returned value：
**********************************************************/
void EPMW_INIT(void)
{

}

void START_INIT(void)
{
	SWDTEN = 0;	//软件关闭看门狗
	INTEDG = 0; //下降沿触发中断
	INTF = 0;
	INTE = 1;
	GIE = 1;
	while(!GIE) CLRWDT();
	SLEEP();
}
/*
void ENABLE_INTERRUPT(void)
{
	TMR1IF = 0;                       //清TIMER1中断标志位
	TMR1IE = 1;                       //使能TIMER1中断
    PEIE = 1;
	GIE = 1;                        //开启总中断
}
*/

void WEEKUP_SETTING(void)
{
	LOAD_RUNSTATUS();
	LEDStatus.SBinkNum = (URunStatus.RunStatus.OffTimerGear*2+1);
	SET_TIMER_RESET();
	SET_EPMW_ON();

}

void LED_BINK(void)
{
	unsigned int LEDT1 = 0;
	unsigned int LEDT2 = 0;
	unsigned int LEDInterval = 0;

	switch(URunStatus.RunStatus.OffTimerGear)
	{
		case 0:
			LEDT1 = LEDGEAR1T1;
			LEDT2 = LEDGEAR1T2;
			break;
		case 1:
			LEDT1 = LEDGEAR2T1;
			LEDT2 = LEDGEAR2T2;
			break;
		case 2: 
			LEDT1 = LEDGEAR3T1;
			LEDT2 = LEDGEAR3T2;
			break;
		default:
			LEDT1 = LEDGEAR1T1;
			LEDT2 = LEDGEAR1T2;
	}
	if (LEDStatus.IBinkNum == 0)
	{
		if(LEDStatus.SBinkNum == 0) {LEDStatus.SBinkNum = (URunStatus.RunStatus.OffTimerGear*2+1)};
		if((((LEDStatus.SBinkNum+1)%2)==0) && (GET_TIME_DIF(LEDTimer)/125 > LEDT))
		{
			LEDTimer = GET_NOW_TIME();
			SET_INDICATOR_LED;
			LEDT = LEDT1
			LEDStatus.SBinkNum -= 1;
		}
		else if ((((LEDStatus.SBinkNum+1)%2)==1) && (GET_TIME_DIF(LEDTimer)/125 > LEDT))
		{
			LEDTimer = GET_NOW_TIME();
			CLEAR_INDICATOR_LED;
			if(SBinkNum == 0)
			{LEDTimer = LEDINTERVAL;}
			else
			{LEDT = LEDT2;}
			LEDStatus.SBinkNum -= 1;
		}

	}
	else
	{
		if((((LEDStatus.IBinkNum+1)%2)==0) && (GET_TIME_DIF(LEDTimer)/125 > LEDT))
		{
			LEDTimer = GET_NOW_TIME();
			SET_INDICATOR_LED;
			LEDT = LEDT1
			LEDStatus.IBinkNum -= 1;
		}
		else if((((LEDStatus.IBinkNum+1)%2)==1) && (GET_TIME_DIF(LEDTimer)/125 > LEDT))
		{
			LEDTimer = GET_NOW_TIME();
			CLEAR_INDICATOR_LED;
			if(IBinkNum == 0)
			{LEDTimer = LEDINTERVAL;}
			else
			{LEDT = LEDT2;}
			LEDStatus.IBinkNum -= 1;
		}
	}
}

void BUTTON_SCAN(void)
{

}

void LOW_POWER_CHECK(void)
{
	if(LVDW==1) LowPower = 1;
}

void SET_EPMW_ON(void)
{
	TRISC |= 0B00001000;
	PR2 = 99;
	EPWMR1L = 50;
	EPWM1CON = 0B10001100; //PWM模式,半桥输出
	TMR2IF = 0;
	T2CON = 0B00000100;
	while(TMR2IF==0) CLRWDT();
	PWM1CON = 0B00010000; 
	EPWM1AUX = 0B10001000;
	TRISC &= 0B11110111;
}

void SET_EPMW_OFF(void)
{
	TRISC 	|= 0B00001000;
	WPUC	|= 0B00001000;
	EPWM1CON = 0B00000000;; 
}

void SET_TIMER_RESET(void)
{
	TMR1H = 0x00;
    TMR1L = 0x00; 
	TMR1IF = 0;                       //清TIMER1中断标志位
	TMR1IE = 1;                       //使能TIMER1中断
    PEIE = 1;
	GIE = 1;                        //开启总中断
}

void SLEEP_TIMER(void)
{

}

void SET_SLEEP(void)
{
	SAVE_RUNSTATUS();
	TMR1IE = 0;
	INTEDG = 0; //下降沿触发中断
	INTF = 0;
	INTE = 1;
	GIE = 1;
	while(!GIE) CLRWDT();
	SLEEP();
}



unsigned int GET_NOW_TIME(void)
{
	unsigned int NowTime = (unsigned int)(TMR1H << 8);
	NowTime += TMR1L;
	return NowTime; 
}

unsigned int GET_TIME_DIF(unsigned int LastTime)
{
	unsigned int NowTime = (unsigned int)(TMR1H << 8);
	NowTime += TMR1L;
	if (LastTime < NowTime)
	{
		return (NowTime - LastTime);
	}
	else
	{
		return (0xFFFF - NowTime - LastTime);
	}
}

void LOAD_RUNSTATUS(void)
{
	WRITE_EEPROM(0xFF,0xAA);			//在未使用到的随意一个地址写两次0xAA
	WRITE_EEPROM(0xFF,0xAA);			//如果程序有读写EEPROM,此操作必须执行 
	URunStatus.RunStatusChar = READ_EEPROM(RunStatusEEAddr);
}

void SAVE_RUNSTATUS(void)
{
	WRITE_EEPROM(RunStatusEEAddr, URunStatus.RunStatusChar);
}


/**********************************************************
*函数名:READ_EEPROM
*功能:读取EEPROM数据
*输入参数:EEAddr读EEPROM的目标地址
*返回参数:ReEepromData读EEPROM的目标地址的数据
**********************************************************/
unsigned char READ_EEPROM(unsigned char EEAddr)
{
	unsigned char ReEepromData;

	EEADR = EEAddr;    
	RD = 1;
	ReEepromData = EEDAT;
	RD = 0;
	return ReEepromData;
}

/**********************************************************
*函数名:READ_EEPROM
*功能:读取EEPROM数据
*输入参数:EEAddr读EEPROM的目标地址
*返回参数:ReEepromData读EEPROM的目标地址的数据
**********************************************************/
void WRITE_EEPROM(unsigned char EEAddr,unsigned char EEData)
{
	GIE = 0;            //写EEPROM关闭中断
	while(GIE) CLRWDT();//等待GIE为0
	EEADR = EEAddr;
	EEDAT = EEData;
	EEIF = 0;
	EECON1 = 0x34;
	WR = 1;
	while(WR) CLRWDT();	//等待写EEPROM完成,如果有开启看门狗且看门狗溢出时间设置在小于等于2ms.
                        //需要清狗,因为写EEPOM大概需要2ms的时间
	GIE = 1;
}


/**********************************************************
*Funtion name：main
*parameters：
*returned value：
**********************************************************/
main()
{
#asm
	MOVLW		0x07			// 系统设置不可以删除和修改
	MOVWF		0x19			// 系统设置不可以删除和修改
#endasm

	LOW_POWER_INIT();
	LED_INIT();
	BUTTON_INIT();
	TIMER_INIT();
	START_INIT();

	while(1)
	{
		LOW_POWER_CHECK();
		LED_BINK();
		BUTTON_SCAN();
	}

}


