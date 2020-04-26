/************************************************************************************
 *
 * 代码与注释：刘家辉
 * 日期：2020/4/9
 *
 * 一些关于SIM868的函数
 *
 * 如有任何疑问，请发送邮件到： liujiahiu@qq.com
*************************************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266

#include "User_SIM868.h"
#include "Private_Timer.h"
#include <libmaple/iwdg.h>
#include <TinyGsmClient.h>
#include <Arduino_JSON.h>
#include "Private_RTC.h"
#include "User_CMDprocess.h"
#include "Response_receipt.h"
#include "Memory.h"

SIM868 Sim868;

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

String IMEI = "";//国际移动设备识别码
String SIMCCID = "";//sim卡的识别标识.
String LOCData = "";//LOCData
String SoftwareVer = "V3.0.0";//软件版本
String HardwareVer = "V1.0.0";//硬件版本
int CSQ = 0;//信号质量
char cmd16array[16] = {0};
const char *cmdarray;
// const char *cmdarray = cmd16array;
const unsigned int OUT_NUM_LIST[MAX_OUT_NUM] = {KCZJ1, KCZJ2};
int Decice_Mode = 0;//设备工作模式
int Decice_States = 0;//设备工作状态

/*
 @brief     : SIM868的初始化（电源的开启，启动或关闭GPS电源，配置GPS参数）
 @param     : None
 @return    : None
 @Called function：
 1.modem.GPS_PWR_CONTROL(ON)    Boot or Close GPS POWER
 【sendAT(GF("+CGNSPWR=1"))】
 2.modem.GPS_Set_parameter()    config GPS  Parameter
 【sendAT(GF("+CGNSTST=0"))、sendAT(GF("+CGNSIPR=115200"))】
 */
void SIM868::Init(void)
{
    SerialAT.begin(9600);
    Sim868.PWR_CON();//
    // Set GSM module baud rate
    delay(3000);
    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println(F("Initializing modem..."));
    modem.restart();
    //打开GPS电源
    modem.GPS_PWR_CONTROL(ON);//启动或关闭GPS电源
    delay(2000);
    modem.GPS_Set_parameter();//配置GPS参数
    delay(1000);
    LBS_Connect_flag = false;
    GSM_Search_Net_Flag = false;
    GSM_Enter_Net_flag = false;
    //Unlock your SIM card with a PIN
    //modem.simUnlock("1234");

    Sever_Connect_flag = false;
}

/*
 @brief     : SIM868电源开启
 @param     : None
 @return    : None
 @Called function：None
 */
void SIM868::PWR_CON(void)
{
    GPRS_PWRKEY_LOW;
    delay(1500);
    GPRS_PWRKEY_HIGH;
}

/*
 @brief     : SIM868得到电压
 @param     : None
 @return    : uint16_t(unsigned short)
 @Called function：
 1.modem.getBattVoltage()   Battery functions
 【sendAT(GF("+CBC"))】
 float vBatt = modem.getBattVoltage() / 1000.0;
 */
uint16_t SIM868::getBattVol(void)
{
    return modem.getBattVoltage();
}

/*
 @brief     : SIM868搜索网络
 @param     : None
 @return    : None
 @Called function：
 1.modem.waitForNetwork()   Network Registration
 【sendAT(GF("+CREG?"))】
 */
bool SIM868::Search_Net(void)//注册网络  
{
    if (GSM_Search_Net_Flag == false)
    {
		Serial.println("");
        Serial.print(F("Waiting for network..."));
        if (!modem.waitForNetwork())
        {
            Serial.println(" fail <SIM868::Search_Net()>");
            delay(10000);
            GSM_Search_Net_Flag = false;
            return false;
        }
        GSM_Search_Net_Flag = true;
        Serial.println("Search GSM Network OK <SIM868::Search_Net()>");
        return true;
    }
    return true;
}

/*
 @brief     : SIM868接入网络
 @param     : None
 @return    : None
 @Called function：
1.modem.gprsConnect(apn, user, pass)    GPRS functions
【sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"')、sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""))】
 */
bool SIM868::Access_Net(void)
{
    if (GSM_Enter_Net_flag == false && GSM_Search_Net_Flag == true)
    {
		Serial.println("");
        Serial.print(F("Connecting to "));
        Serial.print(apn);
        if (!modem.gprsConnect(apn, user, pass))
        {
            Serial.println(" Fail <SIM868::Access_Net()>");
            delay(10000);
            GSM_Enter_Net_flag = false;
            return false;
        }

        Serial.println(" Success <SIM868::Access_Net()>");
        GSM_Enter_Net_flag = true;
        //send SMS
        //modem.sendSMS("+8613577182976", "this is SIM868");
        return true;
    }
    return true;
}

/*
 @brief     : SIM868连接基站定位服务
 @param     : None
 @return    : None
 @Called function：
 1.modem.LBS_Connect()  基站定位连接LBS
 【AT指令调用过多，自行查阅】
 */
void SIM868::Connect_Station_location_Service(void)
{
    if (GSM_Search_Net_Flag==true && LBS_Connect_flag==false)
    {
		Serial.println("");
        if (modem.LBS_Connect() == true)
        {
			Serial.println("LBS Connect OK! <SIM868::Connect_Station_location_Service()>");
            LBS_Connect_flag = true;
        }
        else
        {
            Serial.println("LBS Connect Fail! <SIM868::Connect_Station_location_Service()>");
            LBS_Connect_flag = false;
        }
    }
}

/*
 @brief     : SIM868连接到服务器
 @param     : None
 @return    : None
 @Called function：
 1.client.connect(server, port) 
 【AT指令调用过多，自行查阅】
 2.modem.gprsDisconnect()   gprs断开
【endAT(GF("+CIPSHUT"))】
 3.modem.getSignalQuality() 得到的信号质量
 【sendAT(GF("+CSQ"))】
 4.modem.getIMEI()  得到IMEI
 【sendAT(GF("+GSN"))】 
 5.modem.getSimCCID()   得到SimCCID
 【sendAT(GF("+CCID"))】
 */
bool SIM868::Connect_Server(void)
{
    if (Sever_Connect_flag == false)
	{
		Serial.println("");
		//连接基站定位服务连接基站定位服务
		Serial.print(String("Connecting to Server:") + server);
		if (!client.connect(server, port))
		{
			Serial.println(" fail <SIM868::Connect_Server()>");
			delay(10000);
			Sever_Connect_flag = false;
			Try_Connect_Sever_Num++;
			//连接10次都连接不上服务器，重新入网
			if (Try_Connect_Sever_Num > 10)
			{
				Try_Connect_Sever_Num = 0;
				GSM_Search_Net_Flag = false;
				GSM_Enter_Net_flag = false;
				modem.gprsDisconnect();
				LBS_Connect_flag = false;
			}
			return false;
		}
		else
		{
			Serial.println(" OK! <SIM868::Connect_Server()>");
			Try_Connect_Sever_Num = 0;
			Sever_Connect_flag = true;
			
			Serial.println("GSM Enter network OK");

			// char string[25];
			// sprintf(string, "\nCSQ:%d\r", CSQ);
			// Serial.println(string);
			// CSQ = 0;

			//发送连接信息至服务器
			CSQ = modem.getSignalQuality();
			Serial.println(String("CSQ:") + CSQ);
			
			IMEI = modem.getIMEI();
			Serial.println("IMEI:" + IMEI);

			SIMCCID = modem.getSimCCID();
			Serial.println("SIMCCID:" + SIMCCID);

			if (LBS_Connect_flag == true)
			{
				//0,102.654510,25.064799,550,17/09/14,11:20:30
				LOCData = modem.Get_LOCData();
				Serial.println("LOCData:" + LOCData);
				LBS_Connect_flag = false; //
			}
			Serial.println("-------");

			// Receipt.Receipt_of_Online(CSQ,IMEI,SIMCCID,LOCData,MAX_OUT_NUM,SoftwareVer,HardwareVer);
			Receipt.Receipt_of_Online();//上线报告

			// // JSON.stringify(myVar) can be used to convert the json var to a String
			// String jsonString = JSON.stringify(myObject);
			// Serial.println("JSON.stringify(myObject) = " + jsonString);
			// // Serial.println(jsonString);
			// SendDataToSever(jsonString);

			IMEI.trim();//删除头尾空白符的字符串。
			// Serial.println("IMEI:" + IMEI);
			SIMCCID.trim();//删除头尾空白符的字符串。
			// Serial.println("SIMCCID:" + SIMCCID);
			LOCData.trim();//删除头尾空白符的字符串。
			// Serial.println("LBSData:" + LOCData);

			Serial.println("END <SIM868::Connect_Server()>");
            return true;
		}
	}
    return true;
}

/*
 @brief     : SIM868客户端检查连接状态
 @param     : None
 @return    : None
 @Called function：
 1.client.connected()
 【】
 2.client.stop()
 【sendAT(GF("+CIPCLOSE="), mux)】
 */
void SIM868::Client_Check_Connection()
{
    if (!client.connected())
	{
		if (Sever_Connect_flag == true)
		{
			Sever_Connect_flag = false;
			Serial.println("");
			Serial.println("Sever_Connect_flag = false <SIM868::Client_Check_Connection()>");
			client.stop();
		}
	}
}

/*
 @brief     : SIM868客户端接收命令
 @param     : None
 @return    : None
 @Called function：
 1.client.connect(server, port) 
 【AT指令调用过多，自行查阅】
 2.modem.gprsDisconnect()   gprs断开
【endAT(GF("+CIPSHUT"))】
 3.modem.getSignalQuality() 得到的信号质量
 【sendAT(GF("+CSQ"))】
 4.modem.getIMEI()  得到IMEI
 【sendAT(GF("+GSN"))】 
 5.modem.getSimCCID()   得到SimCCID
 【sendAT(GF("+CCID"))】
 */
void SIM868::Client_ReceiveCMD()
{
    unsigned long timeout = millis();
	while (client.connected() && millis() - timeout < 2000)
	{
		// Print available data
		if (client.available())
		{
			String Res = client.readStringUntil('\n');
			Serial.println("");
			Serial.println("Get information sent by the Server... ");
			Serial.println("Res:" + Res + "\n");Serial.flush();

			Sim868.ReceiveCMD_Analysis(Res);//SIM868接收命令处理

			// Res.trim();//删除头尾空白符的字符串。
		}
	}
}

/*
 @brief     : SIM868接收命令处理
 @param     : String
 @return    : None
 @Called function：
 1.client.write((unsigned char *)sendBuf, datalength)
 【AT指令调用过多，自行查阅】
 */
void SIM868::ReceiveCMD_Analysis(String res)
{
	if (res.equals("") == false)
	{
		JSONVar cmdObject = JSON.parse(res);//把字符串转换为JSON对象
		// JSON.typeof(jsonVar) can be used to get the type of the var
		/* typeof运算符的返回类型为字符串，值包括如下几种： 
		1. 'undefined'              	--未定义的变量或值
 		2. 'boolean'                 	--布尔类型的变量或值
 		3. 'string'                  	--字符串类型的变量或值
 		4. 'number'                  	--数字类型的变量或值
 		5. 'object'                    	--对象类型的变量或值，或者null(这个是js历史遗留问题，将null作为object类型处理)
 		6. 'function'                 	--函数类型的变量或值*/
		if (JSON.typeof(cmdObject) == "undefined")
		{
			Serial.println("Parsing input failed! <SIM868::ReceiveCMD_Processing()>");
			return;
		}
		// Serial.println(); // prints: object
		Serial.println("JSON.typeof(myObject) = " + JSON.typeof(cmdObject));

		// myObject.hasOwnProperty(key) checks if the object contains an entry for key判断自身属性是否存在
		if (cmdObject.hasOwnProperty("rtc"))
		{
			ReviceServerTimeoutNum = 0;
			Serial.println("");
			Serial.print("cmdObject[\"rtc\"] = ");
			Serial.println(cmdObject["rtc"]);
			
			//String DataString = JSON.stringify(cmdObject["rtc"]);
			#if RTC_FUN
			const char *DataArray = (const char *)cmdObject["rtc"];

			cmd.Save_Rtc(DataArray);//

			// noInterrupts();
			// time_t Alarm_Time = 0;
			// Alarm_Time = InRtc.getTime();
			// InRtc.createAlarm(RTC_Interrupt, Alarm_Time + 60);
			// interrupts();
			#endif
		}
		if (cmdObject.hasOwnProperty("DONum"))//单控
		{
			if (cmdObject.hasOwnProperty("sta"))
			{
				int Which_DO = (int)cmdObject["DONum"];
				int Sta = (int)cmdObject["sta"];

				cmd.Individual_Control(Which_DO,Sta);//单独控制
			}
		}
		else if (cmdObject.hasOwnProperty("DO16Sta"))//群控
		{
			Serial.print("cmdObject[\"DO16Sta\"] = ");
			Serial.println(cmdObject["DO16Sta"]);
			cmdarray = (const char *)cmdObject["DO16Sta"];

			cmd.Batch_Control(cmdarray);//批量控制
		}
		else if (cmdObject.hasOwnProperty("Mode"))//模式控制
		{
			Decice_Mode = (int)cmdObject["Mode"];
			Decice_Timing_Mode.Save_DeviceMode((unsigned char)Decice_Mode);//保存设备状态至EEP

			switch (Decice_Mode)
			{
				case 0x00:
				{
					Serial.println("Enter Stop Mode");
					break;
				}
				case 0x01:
				{
					Serial.println("Enter General Control Mode");
					cmd.General_Control_Mode(res);//通用控制模式
					break;
				}
				case 0x02:
				{	
					Serial.println("Enter Weekly Control Mode");
					break;
				}
				default:
				{
					Serial.println("Non existent Decice_Mode!");Serial.println(String("Decice_Mode = ") + Decice_Mode);
					break;
				}
			}
		}
		else
		{
			Serial.println("Instruction error!!! nonexistent instruction");
		}
	}
}

/*
 @brief     : SIM868发送数据至服务器
 @param     : String
 @return    : None
 @Called function：
 1.client.write((unsigned char *)sendBuf, datalength)
 【AT指令调用过多，自行查阅】
 */
bool SIM868::SendDataToSever(String string)
{
	char *sendBuf;
	int datalength = string.length() + 1;
	sendBuf = (char *)malloc(datalength);
	string.toCharArray(sendBuf, datalength);
	client.write((unsigned char *)sendBuf, datalength);
	free(sendBuf);
	return true;
}

void SIM868::Send_Heartbeat_Regularly(void)
{
    // Serial.println(">>>begin to Send Heartbeat Regularly");
    //定时发送心跳包
    if (SendHeatBeatFlag == true && Sever_Connect_flag == true)
    {
		Serial.println("");
		Serial.println("Send Heatbeat! <SIM868::Send_Heartbeat_Regularly()>");
        SendHeatBeatFlag = false;

        //client.print(jsonString1);
        if (client.connected())
        {
			Receipt.Heartbeat_Package_Receipt();//心跳包回执
        }
        ReviceServerTimeoutNum++;
        Serial.println(String("ReviceServerTimeoutNum:") + ReviceServerTimeoutNum);
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
    // Serial.println("<<<end to Send Heartbeat Regularly");
}
