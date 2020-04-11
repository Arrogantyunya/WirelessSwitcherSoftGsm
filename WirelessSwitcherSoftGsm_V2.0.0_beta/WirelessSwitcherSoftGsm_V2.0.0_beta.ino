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

#define RTC_FUN true

#if RTC_FUN
RTClock InRtc(RTCSEL_LSE); // initialise RTC
#endif

bool SendDataToSever(String string);
void RTC_Interrupt(void);

unsigned int TimeSec = 0;				//运行超时计数
bool SendHeatBeatFlag = false;			//发送心跳包标志位
unsigned int GPRSV = 0;					//GPRS的电压
unsigned int Try_Connect_Sever_Num = 0; //尝试连接服务器次数
char cmd16array[16] = {0};
const char *cmdarray = cmd16array;
char Toggle = 0; //状态灯闪灭位翻转
unsigned int ReviceServerTimeoutNum = 0;

const unsigned int OUT_NUM_LIST[MAX_OUT_NUM] = {RELAY_OUT1_PIN, RELAY_OUT2_PIN};

#if RTC_FUN
UTCTimeStruct RtcTime = {20, 20, 4, 9, 15, 16, 30};
#endif

static const unsigned char SoftWareVer[] = "V1.0.1";

void setup()
{
	// Set console baud rate
	afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);

	Some_Peripheral.Peripheral_GPIO_Pinmode();//设置外设的GPIO的引脚模式
	Some_Peripheral.Peripheral_GPIO_Config();//外设的GPIO的初始状态

	Serial.begin(9600);

	Sim868.Init(); //初始化

	//初始化定时器2
	Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
	Timer2.setPeriod(1000000); // in microseconds，1S
	Timer2.setCompare1(1);	   // overflow might be small
	Timer2.attachCompare1Interrupt(Time2_Handler);

#if RTC_FUN
	//初始化RTC闹钟
	noInterrupts();
	time_t Alarm_Time = 0;
	Alarm_Time = InRtc.getTime();
	InRtc.createAlarm(RTC_Interrupt, Alarm_Time + 60);
	interrupts();
#endif
}

void loop()
{
	String IMEI;//国际移动设备识别码
	String SIMCCID;//sim卡的识别标识
	int CSQ = 0;

	// Get GPRS Moudle Voltage
	GPRSV = Sim868.getBattVoltage();//得到电压
	Serial.print(String("GPRS Moudle Vol:") + GPRSV + "mv");

	Sim868.Search_Net(); //GSM搜索网络函数（带return）

	Sim868.Access_Net(); //GSM进入网络函数（带return）

	//连接基站定位服务
	Sim868.Connect_Station_location_Service();

	//String GPSData =modem.GPS_Get_Data();
	//Serial.println("GPS Data:" + GPSData + "\r\n");
	//SerialAT.println("AT+CGNSINF\r\n");
	// delay(2000);

	
	Sim868.Connect_Server();//连接到服务器(带return)
	
	// Make a HTTP GET request:
	//AT+OUT:1=ON

	unsigned long timeout = millis();
	while (client.connected() && millis() - timeout < 5000L)
	{
		// Print available data
		if (client.available())
		{
			String res = client.readStringUntil('\n');
			if (res.equals("") == false)
			{
				JSONVar cmdObject = JSON.parse(res);//解析json（数组或者对象）字符串
				// JSON.typeof(jsonVar) can be used to get the type of the var
				if (JSON.typeof(cmdObject) == "undefined")
				{
					Serial.println("Parsing input failed!");
					return;
				}
				Serial.print("JSON.typeof(myObject) = ");
				Serial.println(JSON.typeof(cmdObject)); // prints: object

				// myObject.hasOwnProperty(key) checks if the object contains an entry for key
				if (cmdObject.hasOwnProperty("time"))
				{
					ReviceServerTimeoutNum = 0;
					Serial.print("cmdObject[\"time\"] = ");
					Serial.println(cmdObject["time"]);
					//20200306183358
					const char *DataArray = (const char *)cmdObject["time"];
					//String DataString = JSON.stringify(cmdObject["time"]);
#if RTC_FUN
					RtcTime.year = (DataArray[0] - '0') * 1000 + (DataArray[1] - '0') * 100 + (DataArray[2] - '0') * 10 + (DataArray[3] - '0');
					RtcTime.month = (DataArray[4] - '0') * 10 + (DataArray[5] - '0');
					RtcTime.day = (DataArray[6] - '0') * 10 + (DataArray[7] - '0');

					RtcTime.hour = (DataArray[8] - '0') * 10 + (DataArray[9] - '0');
					RtcTime.minutes = (DataArray[10] - '0') * 10 + (DataArray[11] - '0');
					RtcTime.seconds = (DataArray[12] - '0') * 10 + (DataArray[13] - '0');

					UTCTime CurrentSec = osal_ConvertUTCSecs(&RtcTime);
					InRtc.setTime(CurrentSec);
					noInterrupts();
					time_t Alarm_Time = 0;
					Alarm_Time = InRtc.getTime();
					InRtc.createAlarm(RTC_Interrupt, Alarm_Time + 60);
					interrupts();
#endif
				}
				if (cmdObject.hasOwnProperty("DO16Sta"))
				{
					Serial.print("DO16Sta[\"DO16Sta\"] = ");
					//char cmd=cmdObject["DO16Sta"];
					//String CmdString = JSON.stringify(cmdObject["DO16Sta"]);
					//Serial.println(CmdString);
					int i = 0;
					//const char cmdarray[16]={0};
					cmdarray = (const char *)cmdObject["DO16Sta"];
					//CmdString.toCharArray(cmdarray, 16);
					for (i = 0; i < 16; i++)
					{
						if (cmdarray[i] == 0x31)
						{
							//Serial.println(i+"channel Open");
							switch (i)
							{
							case 0:
								RELAY_OUT1_ON;
								break;
							case 1:
								RELAY_OUT2_ON;
								break;
							default:
								break;
							}
						}
						else if (cmdarray[i] == 0x30)
						{
							//Serial.println(i+"channel close");
							switch (i)
							{
							case 0:
								RELAY_OUT1_OFF;
								break;
							case 1:
								RELAY_OUT2_OFF;
								break;
							default:
								break;
							}
						}
						else
						{
							Serial.println("error!");
							//Serial.println(cmdarray[i]);
						}
						delay(10);
					}
					memset(cmd16array, '0', sizeof(cmd16array));
					//读输出IO的状态
					for (int i = 0; i < MAX_OUT_NUM; i++)
					{
						if (digitalRead(OUT_NUM_LIST[i]) == LOW)
						{
							cmd16array[i] = '1';
						}
						else if (digitalRead(OUT_NUM_LIST[i]) == HIGH)
						{
							cmd16array[i] = '0';
						}
					}

					//发送应答
					JSONVar AckObject;
					AckObject["IMEI"] = IMEI;
					AckObject["DO16Sta"] = cmd16array;
					Serial.println(IMEI);
					Serial.println(cmdarray);
					Serial.print("AckObject.keys() = ");
					Serial.println(AckObject.keys());
					// JSON.stringify(myVar) can be used to convert the json var to a String
					String jsonString1 = JSON.stringify(AckObject);
					Serial.print("JSON.stringify(myObject) = ");
					Serial.println(jsonString1);
					// client.print(jsonString1);
					char *sendBuf;
					int datalength = jsonString1.length() + 1;
					sendBuf = (char *)malloc(datalength);
					jsonString1.toCharArray(sendBuf, datalength);
					client.write((unsigned char *)sendBuf, datalength);
					free(sendBuf);
				}
				if (cmdObject.hasOwnProperty("DONum"))
				{
					int ChNum = (int)cmdObject["DONum"];
					if (cmdObject.hasOwnProperty("sta"))
					{
						int sta = (int)cmdObject["sta"];
						if (sta == 1)
						{
							switch (ChNum)
							{
							case 255:
								RELAY_OUT1_ON;
								RELAY_OUT2_ON;
								break;
							case 1:
								RELAY_OUT1_ON;
								break;
							case 2:
								RELAY_OUT2_ON;
								break;
							default:
								break;
							}
						}
						else if (sta == 0)
						{
							switch (ChNum)
							{
							case 255:
								RELAY_OUT1_OFF;
								RELAY_OUT2_OFF;
								break;
							case 1:
								RELAY_OUT1_OFF;
								break;
							case 2:
								RELAY_OUT2_OFF;
								break;
							default:
								break;
							}
						}

						//发送应答
						JSONVar AckObject;
						AckObject["IMEI"] = IMEI;
						AckObject["DONum"] = ChNum;
						AckObject["sta"] = sta;
						Serial.print("AckObject.keys() = ");
						Serial.println(AckObject.keys());
						// JSON.stringify(myVar) can be used to convert the json var to a String
						String jsonString1 = JSON.stringify(AckObject);
						Serial.print("JSON.stringify(myObject) = ");
						Serial.println(jsonString1);
						//client.print(jsonString1);
						SendDataToSever(jsonString1);
					}
				}
			}
			res.trim();
		}

		//定时发送心跳包
		if (SendHeatBeatFlag == true && Sever_Connect_flag)
		{
			SendHeatBeatFlag = false;
			JSONVar AckObject;
			memset(cmd16array, 0, sizeof(cmd16array));
			AckObject["IMEI"] = IMEI;
			AckObject["DO16Sta"] = cmd16array;
			Serial.print("AckObject.keys() = ");
			Serial.println(AckObject.keys());
			// JSON.stringify(myVar) can be used to convert the json var to a String
			String jsonString1 = JSON.stringify(AckObject);
			Serial.print("JSON.stringify(myObject) = ");
			Serial.println(jsonString1);
			//client.print(jsonString1);
			if (client.connected())
			{
				SendDataToSever(jsonString1);
			}
			ReviceServerTimeoutNum++;
			Serial.print("ReviceServerTimeoutNum:");
			Serial.println(ReviceServerTimeoutNum);
			//如果接收服务器数据次数超时
			if (ReviceServerTimeoutNum >= 10)
			{
				ReviceServerTimeoutNum = 0;
				if (Sever_Connect_flag == true)
				{
					Sever_Connect_flag = false;
					client.stop();
				}
			}
		}

		timeout = millis();
	}

	if (!client.connected())
	{
		if (Sever_Connect_flag == true)
		{
			Sever_Connect_flag = false;
			client.stop();
		}
	}
}

bool SendDataToSever(String string)
{
	char *sendBuf;
	int datalength = string.length() + 1;
	sendBuf = (char *)malloc(datalength);
	string.toCharArray(sendBuf, datalength);
	client.write((unsigned char *)sendBuf, datalength);
	free(sendBuf);
	return true;
}
/*
 *brief   : 定时器2中断函数
 *para    : 无
 *return  : 无
*/
void Time2_Handler(void)
{
	//Toggle ^= 1;
	//digitalWrite(RELAY_OUT1_PIN, Toggle); //状态灯闪烁
	TimeSec++;
	//如果运行超时，复位
	if (TimeSec >= 30)
	{
		TimeSec = 0;
		SendHeatBeatFlag = true;
	}
}

#if RTC_FUN
void RTC_Interrupt(void)
{
	rtc_detach_interrupt(RTC_ALARM_SPECIFIC_INTERRUPT);
	Serial.println("RTC alarm interrupt");
}
#endif
