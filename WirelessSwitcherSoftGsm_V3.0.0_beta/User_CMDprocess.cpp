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
// #include <Arduino_JSON.h>
#include "Private_RTC.h"
#include "Response_receipt.h"

CMD cmd;

/*
 @brief     : 保存rtc
 @param     : None
 @return    : None
 @Called function：None
 */
void CMD::Save_Rtc(const char * rtc)
{
    unsigned char RTC[7] = {0};

    for (size_t i = 0; i < 7; i++)
    {
        RTC[i] = (rtc[2*i] - '0') * 10 + (rtc[(2*i)+1] - '0');
    }
    Private_RTC.Update_RTC(RTC);
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