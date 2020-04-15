/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * 
 *
 * TinyGSM Getting Started guide:
 *   https://github.com/ynkady/TinyGSM
 *
 **************************************************************/
/*
 使用注意：
 本测试程序适用SIM８６８ 多功能GSM＋GPS模块，使用前需注意以下事项：
 1、此模块适用于兑悦物联小店的GSMandGPS_MDV1.2及以上版本的模块，其它SIM８６８模块需要对IO进行修改
 https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-16679354584.19.5a9c5712xGAkkw&id=567100266868
 ２、本测试程序适用于Arduino UNO、Mega2560官方版本及其它国内各种改进版本，UNO板也可以参考本店的：
 https://item.taobao.com/item.htm?spm=a1z10.3-c-s.w4002-16618966406.83.339d69c4yPsWMK&id=558178885996
 3、使用本测度程序前需下载TinyGSM库，但原来官方的库里不支持基站定位，我们对这个库进行了更改，并上传到Github,可从以下地址下载，并替换原来的库文件
 https://github.com/ynkady/TinyGSM
 Windows的Arduino库文件地址：C:\Users\kady\Documents\Arduino\libraries
 4、GSM模块硬件连接
 4V---->外部电源供电+（3.4~4.2V,推荐4.0V）
 GND--->外部电源-
 GND--->Arduino UNO GND
 PWRKEY-----> D4
 RX---------> D3
 TX---------> D2   
 特别注意：由于GSM模块要搜索移动网络时瞬间电流非常大，可能达到2A，所以GSM模块不能使用ARDUINO开发板上的电源供电，需要使用外部稳定的电源供电，或使用容量大于1500mAh以上的锂电池供电
 GSM模块供电模块可以从我们的小店找到：
 https://item.taobao.com/item.htm?spm=a1z10.3-c-s.w4002-16618966406.41.339d69c4yPsWMK&id=554761942043
 */

// #include <TinyGsmClient.h>
#include <Arduino_JSON.h>
#include <RTClock.h>
#include "User_Clock.h"
#include "fun_periph.h"
#include "User_SIM868.h"
#include "Private_Timer.h"
#include "Private_RTC.h"
#include "public.h"

#define TIMER_TEST 	true

#if RTC_FUN
// RTClock InRtc(RTCSEL_LSE); // initialise RTC
#endif

// void RTC_Interrupt(void);

unsigned int TimeSec = 0;				//运行超时计数
unsigned int GPRSV = 0;					//GPRS的电压

#if RTC_FUN
// UTCTimeStruct RtcTime = {20, 20, 4, 9, 15, 16, 30};
#endif

static const unsigned char SoftWareVer[] = "V2.0.0";
static const unsigned char HardWareVer[] = "V1.0.0";

void setup()
{
	// Set console baud rate
	afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);

	Some_Peripheral.Peripheral_GPIO_Pinmode();//设置外设的GPIO的引脚模式
	Some_Peripheral.Peripheral_GPIO_Config();//外设的GPIO的初始状态

	Serial.begin(9600);

	Sim868.Init(); //初始化

	Timer2_Init();//初始化定时器2
	Start_Timer2();//开始定时器2计数

#if RTC_FUN
	//初始化RTC闹钟
	// noInterrupts();
	// time_t Alarm_Time = 0;
	// Alarm_Time = InRtc.getTime();
	// InRtc.createAlarm(RTC_Interrupt, Alarm_Time + 60);
	// interrupts();
	Serial.println("RTC_FUN");
#endif

	// Get GPRS Moudle Voltage
	GPRSV = Sim868.getBattVol();//得到电压
	Serial.println(String("GPRS Moudle Vol:") + GPRSV + "mv");
}

void loop()
{
	if (!Sim868.Search_Net())//GSM搜索网络函数
	{
		Serial.println("error for <Sim868.Search_Net()>");
		return;
	}
	
	if (!Sim868.Access_Net())//GSM进入网络函数
	{
		Serial.println("error for <Sim868.Access_Net()>");
		return;
	}
	
	//连接基站定位服务
	Sim868.Connect_Station_location_Service();

	//String GPSData =modem.GPS_Get_Data();
	//Serial.println("GPS Data:" + GPSData + "\r\n");
	//SerialAT.println("AT+CGNSINF\r\n");
	// delay(2000);

	if (!Sim868.Connect_Server())//连接到服务器
	{
		Serial.println("error for <Sim868.Connect_Server()>");
		return;
	}
	
	
	// Make a HTTP GET request:
	//AT+OUT:1=ON
	Sim868.Client_ReceiveCMD();//SIM868客户端接收命令
	
	Sim868.Client_Check_Connection();//SIM868客户端检查连接状态

	Sim868.Send_Heartbeat_Regularly();//定时发送心跳包
}

#if RTC_FUN
// void RTC_Interrupt(void)
// {
// 	rtc_detach_interrupt(RTC_ALARM_SPECIFIC_INTERRUPT);
// 	Serial.println("RTC alarm interrupt");
// }
#endif
