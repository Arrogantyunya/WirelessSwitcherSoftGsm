#ifndef _USER_CMDPROCESS_H
#define _USER_CMDPROCESS_H

#include <Arduino.h>
#include "fun_periph.h"


class CMD {
private:
    /* data */
public:
	void Save_Rtc(const char* rtc);//保存RTC
    void Individual_Control(int Which_DO,int Sta);//单独控制
    void Batch_Control(const char* Cmd_data);//批量控制
};

// class SIM868 : public GPRS{
// private:
//     /* data */
// public:
//     void Init(void);            //初始化
// 	void PWR_CON(void);         //
//     uint16_t getBattVol(void);  //得到电压
//     bool Search_Net(void);      //搜索网络
//     bool Access_Net(void);      //连接网络
//     void Connect_Station_location_Service(void);//连接基站定位服务
//     bool Connect_Server(void);//连接到服务器
//     void Client_Check_Connection();//客户端检查连接状态
//     void Client_ReceiveCMD();//客户端接收命令
//     void ReceiveCMD_Analysis(String res);//接收命令处理
//     bool SendDataToSever(String string);//发送数据至服务器
//     void Send_Heartbeat_Regularly();//定时发送心跳包
// };

extern CMD cmd;

#endif