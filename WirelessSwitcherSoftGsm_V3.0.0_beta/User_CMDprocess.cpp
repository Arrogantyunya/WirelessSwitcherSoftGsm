/************************************************************************************
 *
 * 代码与注释：刘家辉
 * 日期：2020/4/9
 *
 * 一些关于指令分析的函数
 *
 * 如有任何疑问，请发送邮件到： liujiahiu@qq.com
*************************************************************************************/
#include "User_CMDprocess.h"
#include "User_SIM868.h"
// #include "Private_Timer.h"
// #include <libmaple/iwdg.h>
// #include <TinyGsmClient.h>
#include <Arduino_JSON.h>
#include "Private_RTC.h"
#include "Response_receipt.h"
#include "Memory.h"

CMD cmd;

/*
 @brief     : 保存rtc
 @param     : None
 @return    : None
 @Called function：None
 */
void CMD::Save_Rtc(const char * rtc)
{
    unsigned char RTC[8] = {0};

    for (size_t i = 0; i < 7; i++)
    {
        RTC[i] = (rtc[2*i] - '0') * 10 + (rtc[(2*i)+1] - '0');
    }
    Private_RTC.Update_RTC(RTC);//写入RTC时间
    Private_RTC.Get_RTC(RTC);//获取芯片RTC时间
    RTC[7] = Private_RTC.RTC_Get_Week(RTC[0]*100+RTC[1],RTC[2],RTC[3]);
    Serial.println(String("WEEK:") + RTC[7]);
    // Serial.println(String("WEEK:") + Private_RTC.RTC_Get_Week(RTC[0]*100+RTC[1],RTC[2],RTC[3]));
    // Serial.println(String("WEEK:") + Private_RTC.WeekDayCount(RTC[0]*100+RTC[1],RTC[2],RTC[3]));
    // Serial.println(String("WEEK:") + Private_RTC.Get_Week(RTC[0]*100+RTC[1],RTC[2],RTC[3]));
}

/*
 @brief     : 单独控制
 @param     : None
 @return    : None
 @Called function：None
 */
void CMD::Individual_Control(int Which_DO,int Sta)
{
    if (Sta == 1)
    {
        switch (Which_DO)
        {
        case 0xFF:
            Some_Peripheral.Set_Relay(0,on);
            Some_Peripheral.Set_Relay(1,on);
            break;
        case 0x01:
            Some_Peripheral.Set_Relay(0,on);
            break;
        case 0x02:
            Some_Peripheral.Set_Relay(1,on);
            break;
        default:
            break;
        }
    }
    else if (Sta == 0)
    {
        switch (Which_DO)
        {
        case 0xFF:
            Some_Peripheral.Set_Relay(0,off);
            Some_Peripheral.Set_Relay(1,off);
            break;
        case 0x01:
            Some_Peripheral.Set_Relay(0,off);
            break;
        case 0x02:
            Some_Peripheral.Set_Relay(1,off);
            break;
        default:
            break;
        }
    }

    Receipt.Receipt_of_Individual_Control(Which_DO,Sta);//单控回执
}

/*
 @brief     : 批量控制
 @param     : None
 @return    : None
 @Called function：None
 */
void CMD::Batch_Control(const char* Cmd_data)
{
    for (size_t i = 0; i < MAX_OUT_NUM; i++)
	{
		if(Cmd_data[i] == 0x31)
		{
			Some_Peripheral.Set_Relay(i,on);
		}
		else if(Cmd_data[i] == 0x30)
		{
			Some_Peripheral.Set_Relay(i,off);
		}
		else
		{
			Serial.println("error!");
			Serial.println(String("Cmd_data[") + i +"]=" + Cmd_data[i]);
		}
	}

    Receipt.Receipt_of_Batch_Control();
}

/*
 @brief     : 通用控制模式
 @param     : None
 @return    : None
 @Called function：None
 */
bool CMD::General_Control_Mode_Save(String res)
{
    /* 5个时间段的开始时间 */
    String Str_General_Control_Mode_Begin;//定义String数组
    char C_General_Control_Mode_Begin[9];//定义数组
    /* 5个时间段的结束时间 */
    String Str_General_Control_Mode_End;//定义String数组
    char C_General_Control_Mode_End[9];//定义数组
    /* 5个时间段的DoUsd */
    String Str_General_Control_Mode_DoUsed;//定义String数组
    char C_General_Control_Mode_DoUsed[17];//定义数组

    JSONVar Json_Time = JSON.parse(res);

	JSONVar J_General_Control_Mode = Json_Time["time"];

    // Serial.print("JSON.typeof(J_General_Control_Mode) = ");
    // Serial.println(JSON.typeof(J_General_Control_Mode));// prints: array

    Serial.println();    
    Serial.print("J_General_Control_Mode.length() = ");
    Serial.println(J_General_Control_Mode.length());// prints: 2
    Serial.println();

    if(J_General_Control_Mode.length() > 5)
    {
        Serial.println("长度超出限制");
        return false;
    }

    /* 清除已经保存的闹钟启动 */
    if(!Decice_Timing_Mode.Clean_Alarm_Used())
    {
        return false;
    }

    /* 此处通过for循环分别对结构体数组内的数组赋值 */
    for (size_t i = 0; i < J_General_Control_Mode.length(); i++)
    {
        //5个时间段的开始时间
        Str_General_Control_Mode_Begin = J_General_Control_Mode[i]["Begin"];
        Str_General_Control_Mode_Begin.toCharArray(C_General_Control_Mode_Begin,Str_General_Control_Mode_Begin.length() + 1);
        strcpy(Alarm_Array[i].Begin,C_General_Control_Mode_Begin);//
        Serial.println(String("Alarm_Array[") + i + "].Begin = " + Alarm_Array[i].Begin);
        
        //5个时间段的结束时间
        Str_General_Control_Mode_End = J_General_Control_Mode[i]["End"];
        Str_General_Control_Mode_End.toCharArray(C_General_Control_Mode_End,Str_General_Control_Mode_End.length() + 1);
        strcpy(Alarm_Array[i].End,C_General_Control_Mode_End);//
        Serial.println(String("Alarm_Array[") + i + "].End = " + Alarm_Array[i].End);

        //5个时间段的DoUsed
        Str_General_Control_Mode_DoUsed = J_General_Control_Mode[i]["DoUsed"];
        Str_General_Control_Mode_DoUsed.toCharArray(C_General_Control_Mode_DoUsed,Str_General_Control_Mode_DoUsed.length() + 1);
        strcpy(Alarm_Array[i].DoUsed,J_General_Control_Mode[i]["DoUsed"]);//
        Serial.println(String("Alarm_Array[") + i + "].DoUsed = " + Alarm_Array[i].DoUsed);

        // 保存闹钟i是否启动的EEPROM
        Decice_Timing_Mode.Save_Alarm_Used(i);
    }

    //保存通用模式的间隔时间至EEP
    Mode1_interval = (int)Json_Time["interval"];
	if(!Decice_Timing_Mode.Save_Mode1_interval((unsigned char)Mode1_interval))
    {
        return false;
    }

    Mode1_RetryCnt = (int)Json_Time["RetryCnt"];
    if(!Decice_Timing_Mode.Save_Mode1_RetryCnt((unsigned char)Mode1_RetryCnt))
    {
        return false;
    }


    return true;
    
    // Serial.print("J_General_Control_Mode[0] = ");
    // Serial.println(J_General_Control_Mode[0]);//{"Begin":"08:00:00","End":"12:00:00","DoUsed":[1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0]}
    // Serial.print("J_General_Control_Mode[1] = ");
    // Serial.println(J_General_Control_Mode[1]);//{"Begin":"12:00:00","End":"16:00:00","DoUsed":[1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0]}
    // Serial.flush();

    // Serial.print("J_General_Control_Mode[0][\"Begin\"] = ");
    // Serial.println(J_General_Control_Mode[0]["Begin"]);
    // Serial.print("J_General_Control_Mode[0][\"End\"] = ");
    // Serial.println(J_General_Control_Mode[0]["End"]);
    // Serial.print("J_General_Control_Mode[0][\"DoUsed\"] = ");
    // Serial.println(J_General_Control_Mode[0]["DoUsed"]);

    // Serial.print("J_General_Control_Mode[1][\"Begin\"] = ");
    // Serial.println(J_General_Control_Mode[1]["Begin"]);
    // Serial.print("J_General_Control_Mode[1][\"End\"] = ");
    // Serial.println(J_General_Control_Mode[1]["End"]);
    // Serial.print("J_General_Control_Mode[1][\"DoUsed\"] = ");
    // Serial.println(J_General_Control_Mode[1]["DoUsed"]);
    // Serial.flush();

    // Serial.print("JSON.typeof(J_General_Control_Mode[1][\"DoUsed\"]) = ");
    // Serial.println(JSON.typeof(J_General_Control_Mode[1]["DoUsed"])); // prints: array
}

/*
 @brief     : 按周控制模式
 @param     : None
 @return    : None
 @Called function：None
 */
bool Week_Control_Mode_Save(String res)
{

}


/*
 @brief     : 停止模式
 @param     : None
 @return    : None
 @Called function：None
 */
bool Stop_Control_Mode_Save(String res)
{

}