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
#include "fun_periph.h"
#include <libmaple/iwdg.h>
#include <TinyGsmClient.h>

SIM868 Sim868;

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void SIM868::Init(void)
{
    SerialAT.begin(9600);
    Sim868.PWR_CON();
    // Set GSM module baud rate
    delay(3000);
    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println(F("Initializing modem..."));
    modem.restart();
    //打开GPS电源
    modem.GPS_PWR_CONTROL(ON);
    delay(2000);
    modem.GPS_Set_parameter();
    delay(1000);
    LBS_Connect_flag = false;
    GSM_Search_Net_Flag = false;
    GSM_Enter_Net_flag = false;
    //Unlock your SIM card with a PIN
    //modem.simUnlock("1234");

    Sever_Connect_flag = false;
}

void SIM868::PWR_CON(void)
{
    GPRS_PWRKEY_LOW;
    delay(1500);
    GPRS_PWRKEY_HIGH;
}

uint16_t SIM868::getBattVoltage(void)
{
    return modem.getBattVoltage();
}

bool SIM868::Search_Net(void)
{
    if (GSM_Search_Net_Flag == false)
    {
        Serial.print(F("Waiting for network..."));
        if (!modem.waitForNetwork())
        {
            Serial.println(" fail");
            delay(10000);
            GSM_Search_Net_Flag = false;
            return false;
        }
        GSM_Search_Net_Flag = true;
        Serial.println("Search GSM Network OK");
        return true;
    }
}

bool SIM868::Access_Net(void)
{
    if (GSM_Enter_Net_flag == false && GSM_Search_Net_Flag == true)
    {
        Serial.print(F("Connecting to "));
        Serial.print(apn);
        if (!modem.gprsConnect(apn, user, pass))
        {
            Serial.println(" Fail");
            delay(10000);
            GSM_Enter_Net_flag = false;
            return false;
        }

        Serial.println(" Success");
        GSM_Enter_Net_flag = true;
        //send SMS
        //modem.sendSMS("+8613577182976", "this is SIM868");
        return true;
    }
}

bool SIM868::Connect_Station_location_Service(void)
{
    if (LBS_Connect_flag == false)
    {
        if (modem.LBS_Connect() == true)
        {
            LBS_Connect_flag = true;
        }
        else
        {
            Serial.println("LBS Connect Fail!");
            LBS_Connect_flag = false;
        }
    }

    String LOCData;
    if (LBS_Connect_flag == true)
    {
        Serial.println("LBS Connect OK!");
        //0,102.654510,25.064799,550,17/09/14,11:20:30
        LOCData = modem.Get_LOCData();
        Serial.println("LBSData:" + LOCData + "\r\n");
        LBS_Connect_flag = false; //
    }
}

bool SIM868::Connect_Server(void)
{
    if (Sever_Connect_flag == false)
	{
		//连接基站定位服务连接基站定位服务
		Serial.print(F("Connecting to Server:"));
		Serial.print(server);
		if (!client.connect(server, port))
		{
			Serial.println(" fail");
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
			Serial.println(" OK!");
			Try_Connect_Sever_Num = 0;
			Sever_Connect_flag = true;
			//发送连接信息组服务器
			CSQ = 0;
			CSQ = modem.getSignalQuality();
			Serial.println("\nGSM Enter network OK\r");
			char string[25];
			sprintf(string, "CSQ:%d\r", CSQ);
			Serial.println(string);
			IMEI = modem.getIMEI();
			Serial.println("IMEI:" + IMEI);
			//Serial.print(CSQ,DEC);
			Serial.println("\r");
			SIMCCID = modem.getSimCCID();
			Serial.println("SIMCCID:" + SIMCCID);
			JSONVar myObject;
			myObject["CSQ"] = CSQ;
			myObject["IMEI"] = IMEI;
			myObject["CCID"] = SIMCCID;
			myObject["LBS"] = LOCData;
			Serial.print("myObject.keys() = ");
			Serial.println(myObject.keys());
			// JSON.stringify(myVar) can be used to convert the json var to a String
			String jsonString = JSON.stringify(myObject);
			Serial.print("JSON.stringify(myObject) = ");
			Serial.println(jsonString);
			SendDataToSever(jsonString);
			IMEI.trim();
			SIMCCID.trim();
			LOCData.trim();
			delay(1000);
            return true;
		}
	}
}
