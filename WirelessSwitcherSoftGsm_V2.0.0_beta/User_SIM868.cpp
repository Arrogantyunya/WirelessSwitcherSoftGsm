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

SIM868 Sim868;

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

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

			JSONVar myObject;
			myObject["CSQ"] = CSQ;
			myObject["IMEI"] = IMEI;
			myObject["CCID"] = SIMCCID;
			myObject["LBS"] = LOCData;
			Serial.print("myObject.keys() = ");
			Serial.println(myObject.keys());

			// JSON.stringify(myVar) can be used to convert the json var to a String
			String jsonString = JSON.stringify(myObject);
			Serial.println("JSON.stringify(myObject) = " + jsonString);
			// Serial.println(jsonString);
			SendDataToSever(jsonString);

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
		delay(250);
		if (client.available())
		{
			String Res = client.readStringUntil('\n');
			Serial.println("");
			Serial.println("Get Client ReceiveCMD... <SIM868::Client_ReceiveCMD()>");
			Serial.println("Res:" + Res);

			Sim868.ReceiveCMD_Processing(Res);//SIM868接收命令处理

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
void SIM868::ReceiveCMD_Processing(String res)
{
	if (res.equals("") == false)
	{
		JSONVar cmdObject = JSON.parse(res);//解析json（数组或者对象）字符串
		// JSON.typeof(jsonVar) can be used to get the type of the var
		if (JSON.typeof(cmdObject) == "undefined")
		{
			Serial.println("Parsing input failed! <SIM868::ReceiveCMD_Processing()>");
			return;
		}
		Serial.println("JSON.typeof(myObject) = " + JSON.typeof(cmdObject));
		// Serial.println(); // prints: object

		// myObject.hasOwnProperty(key) checks if the object contains an entry for key
		if (cmdObject.hasOwnProperty("time"))
		{
			ReviceServerTimeoutNum = 0;
			Serial.print("cmdObject[\"time\"] = ");
			Serial.println(cmdObject["time"]);
			
			//String DataString = JSON.stringify(cmdObject["time"]);
			#if RTC_FUN
			const char *DataArray = (const char *)cmdObject["time"];
			unsigned char RTC[7] = {0};

			for (size_t i = 0; i < 7; i++)
			{
				RTC[i] = (DataArray[2*i] - '0') * 10 + (DataArray[(2*i)+1] - '0');
			}
			Private_RTC.Update_RTC(RTC);

			// noInterrupts();
			// time_t Alarm_Time = 0;
			// Alarm_Time = InRtc.getTime();
			// InRtc.createAlarm(RTC_Interrupt, Alarm_Time + 60);
			// interrupts();
			#endif
		}
		if (cmdObject.hasOwnProperty("DO16Sta"))
		{
			Serial.print("cmdObject[\"DO16Sta\"] = ");
			Serial.println(cmdObject["DO16Sta"]);
			cmdarray = (const char *)cmdObject["DO16Sta"];
			
			for (size_t i = 0; i < MAX_OUT_NUM; i++)
			{
				if(cmdarray[i] == 0x31)
				{
					Some_Peripheral.Set_Relay(i,on);
				}
				else if(cmdarray[i] == 0x30)
				{
					Some_Peripheral.Set_Relay(i,off);
				}
				else
				{
					Serial.println("error!");
					Serial.println(String("cmdarray[") + i +"]=" + cmdarray[i]);
				}
				

				// if (cmdarray[i] == 0x31)
				// {
				// 	//Serial.println(i+"channel Open");
				// 	switch (i)
				// 	{
				// 	case 0:
				// 		RELAY_OUT1_ON;
				// 		break;
				// 	case 1:
				// 		RELAY_OUT2_ON;
				// 		break;
				// 	default:
				// 		break;
				// 	}
				// }
				// else if (cmdarray[i] == 0x30)
				// {
				// 	//Serial.println(i+"channel close");
				// 	switch (i)
				// 	{
				// 	case 0:
				// 		RELAY_OUT1_OFF;
				// 		break;
				// 	case 1:
				// 		RELAY_OUT2_OFF;
				// 		break;
				// 	default:
				// 		break;
				// 	}
				// }
				// else
				// {
				// 	Serial.println("error!");
				// 	//Serial.println(cmdarray[i]);
				// }
				// delay(10);
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

			// 发送群控指令应答
			JSONVar AckObject;
			IMEI = modem.getIMEI();
			AckObject["IMEI"] = IMEI;
			Serial.println(String("IMEI:") + IMEI);
			String DO16Sta(cmd16array);
			AckObject["DO16Sta"] =DO16Sta;
			Serial.println(String("DO16Sta:") + DO16Sta);
			Serial.print("AckObject.keys() = ");
			Serial.println(AckObject.keys());
			// JSON.stringify(myVar) can be used to convert the json var to a String
			String jsonString1 = JSON.stringify(AckObject);
			Serial.print("JSON.stringify(myObject) = ");
			Serial.println(jsonString1);
			// client.print(jsonString1);
			// char *sendBuf;
			// int datalength = jsonString1.length() + 1;
			// sendBuf = (char *)malloc(datalength);
			// jsonString1.toCharArray(sendBuf, datalength);
			// client.write((unsigned char *)sendBuf, datalength);
			// free(sendBuf);
			SendDataToSever(jsonString1);
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
						Some_Peripheral.Set_Relay(0,on);
						Some_Peripheral.Set_Relay(1,on);
						// RELAY_OUT1_ON;
						// RELAY_OUT2_ON;
						break;
					case 1:
						Some_Peripheral.Set_Relay(0,on);
						// RELAY_OUT1_ON;
						break;
					case 2:
						Some_Peripheral.Set_Relay(1,on);
						// RELAY_OUT2_ON;
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
						Some_Peripheral.Set_Relay(0,off);
						Some_Peripheral.Set_Relay(1,off);
						// RELAY_OUT1_OFF;
						// RELAY_OUT2_OFF;
						break;
					case 1:
						Some_Peripheral.Set_Relay(0,off);
						// RELAY_OUT1_OFF;
						break;
					case 2:
						Some_Peripheral.Set_Relay(1,off);
						// RELAY_OUT2_OFF;
						break;
					default:
						break;
					}
				}

				//发送应答
				JSONVar AckObject;
				IMEI = modem.getIMEI();
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
            Sim868.SendDataToSever(jsonString1);
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
