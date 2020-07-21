/************************************************************************************
 *
 * 代码与注释：刘家辉
 * 日期：2020/4/9
 *
 * 一些关于响应回执的函数
 *
 * 如有任何疑问，请发送邮件到： liujiahiu@qq.com
*************************************************************************************/
#include "Response_receipt.h"
// #include "User_CMDprocess.h"
#include "User_SIM868.h"
// #include "Private_Timer.h"
// #include <libmaple/iwdg.h>
// #include <TinyGsmClient.h>
#include <Arduino_JSON.h>
// #include "Private_RTC.h"

RECEIPT Receipt;

/*
 @brief     : 初次连接服务器回执
 @param     : None
 @return    : None
 @Called function：None
 */
// void RECEIPT::Receipt_of_Online(int CSQ,String IMEI,SIMCCID,String LOCData,int MAX_OUT_NUM,String SoftwareVer,String HardwareVer)
void RECEIPT::Receipt_of_Online()
{
    // {"CSQ":14,"IMEI":"867857035021067","CCID":"89860411101870509158","LBS":"3","DOnum":2,"SwVer":"V2.0.0","HwVer":"V1.0.0"}
    
    Serial.println("");
    JSONVar myObject;
    myObject["CSQ"] = CSQ;
    myObject["IMEI"] = IMEI;
    myObject["CCID"] = SIMCCID;
    myObject["LBS"] = LOCData;
    myObject["DOnum"] = MAX_OUT_NUM;
    myObject["SwVer"] = SoftwareVer;
    myObject["HwVer"] = HardwareVer;
    Serial.print("myObject.keys() = ");
    Serial.println(myObject.keys());

    // JSON.stringify(myVar) can be used to convert the json var to a String
    String jsonString = JSON.stringify(myObject);
    Serial.println("JSON.stringify(myObject) = " + jsonString);
    Sim868.SendDataToSever(jsonString);

    Serial.println("<Receipt_of_Online>");
}

/*
 @brief     : 单控回执
 @param     : None
 @return    : None
 @Called function：None
 */
void RECEIPT::Receipt_of_Individual_Control(int DOnum, int sta)
{
    // {"IMEI":"867857035021067","DONum":2,"sta":1}

    Serial.println("");

    JSONVar AckObject;
    // IMEI = modem.getIMEI();
    AckObject["IMEI"] = IMEI;
    AckObject["DONum"] = DOnum;
    AckObject["sta"] = sta;
    Serial.print("AckObject.keys() = ");
    Serial.println(AckObject.keys());
    // JSON.stringify(myVar) can be used to convert the json var to a String
    String jsonString1 = JSON.stringify(AckObject);
    Serial.print("JSON.stringify(myObject) = ");
    Serial.println(jsonString1);
    //client.print(jsonString1);
    Sim868.SendDataToSever(jsonString1);
    Serial.println("<Receipt_of_Individual_Control>");
}

/*
 @brief     : 群控回执
 @param     : None
 @return    : None
 @Called function：None
 */
void RECEIPT::Receipt_of_Batch_Control()
{
    // {"IMEI":"867857035021067","DO16Sta":"0000000000000000"}
    Serial.println("");

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

	// IMEI = modem.getIMEI();
	AckObject["IMEI"] = IMEI;
	Serial.println(String("IMEI:") + IMEI);

	String DO16Sta(cmd16array);
	Serial.println(String("DO16Sta:") + DO16Sta);

	AckObject["DO16Sta"] = DO16Sta;
	// AckObject["DO16Sta"] = cmd16array;
	
	Serial.print("AckObject.keys() = ");
	Serial.println(AckObject.keys());
	// JSON.stringify(myVar) can be used to convert the json var to a String
	String jsonString1 = JSON.stringify(AckObject);//把json格式数据转换为字符串就用这个方法
	Serial.print("JSON.stringify(myObject) = ");
	Serial.println(jsonString1);
	// client.print(jsonString1);
	// char *sendBuf;
	// int datalength = jsonString1.length() + 1;
	// sendBuf = (char *)malloc(datalength);
	// jsonString1.toCharArray(sendBuf, datalength);
	// client.write((unsigned char *)sendBuf, datalength);
	// free(sendBuf);
	Sim868.SendDataToSever(jsonString1);
    Serial.println("<Receipt_of_Batch_Control>");
}

/*
 @brief     : 通用回执
 @param     : None
 @return    : None
 @Called function：None
 */
void RECEIPT::General_Receipt(int Device_mode,int Device_Status)
{
    // {"IMEI":"867857035021067","Mode":0,"Status":1,"DO16Sta":"0000000000000000"}

    Serial.println("");

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

    JSONVar AckObject;
    AckObject["IMEI"] = IMEI;
    AckObject["Mode"] = Device_mode;
    AckObject["Status"] = Device_Status;
    String DO16Sta(cmd16array);
	Serial.println(String("DO16Sta:") + DO16Sta);

	AckObject["DO16Sta"] = DO16Sta;

    Serial.print("AckObject.keys() = ");
	Serial.println(AckObject.keys());
	// JSON.stringify(myVar) can be used to convert the json var to a String
	String jsonString1 = JSON.stringify(AckObject);//把json格式数据转换为字符串就用这个方法
	Serial.print("JSON.stringify(myObject) = ");
	Serial.println(jsonString1);

    Sim868.SendDataToSever(jsonString1);
    Serial.println("<General_Receipt>");
}

/*
 @brief     : 心跳包回执
 @param     : None
 @return    : None
 @Called function：None
 */
void  RECEIPT::Heartbeat_Package_Receipt()
{
    // {"IMEI":"867857035021067"}
    
    Serial.println("");

    JSONVar AckObject;
    // memset(cmd16array, 0, sizeof(cmd16array));
    AckObject["IMEI"] = IMEI;
    // AckObject["DO16Sta"] = cmd16array;
    Serial.print("AckObject.keys() = ");
    Serial.println(AckObject.keys());
    // JSON.stringify(myVar) can be used to convert the json var to a String
    String jsonString1 = JSON.stringify(AckObject);
    Serial.print("JSON.stringify(myObject) = ");
    Serial.println(jsonString1);

    Sim868.SendDataToSever(jsonString1);
    Serial.println("<Heartbeat_Package_Receipt>");
}