#ifndef _USER_SIM868_H
#define _USER_SIM868_H

#include <Arduino.h>
#include "fun_periph.h"

#define RTC_FUN 	false
#define TEST        true


// Your GPRS credentials
// Leave empty, if missing user or pass
//普通GSM 2G网络接入点APN为CMNET
//物联网专用接入点APN为CMIOT
const char apn[] = "CMIOT";
const char user[] = "";
const char pass[] = "";


#if TEST
const char server[] = "211.149.155.108";
const int port = 9000;
#else
const char server[] = "shhoo.cn";
const int port = 6000;
#endif

//以下为服务器地址，需要根据实际要连接的服务器地址填写，注意：必须是公网址哦！
//const char server[] = "211.149.155.108";
//int port = 48979;
//const char server[] = "148.70.126.68";


class GPRS {
private:
    /* data */
public:
	/* data */
};

class SIM868 : public GPRS{
private:
    /* data */
public:
    bool GSM_Enter_Net_flag = false;//进入网络标志
    bool GSM_Search_Net_Flag = false;//GSM搜索网络标志
    bool LBS_Connect_flag = false;//LBS连接标志
    bool Sever_Connect_flag = false;//服务器连接标志位
    unsigned int Try_Connect_Sever_Num = 0; //尝试连接服务器次数
    unsigned int ReviceServerTimeoutNum = 0;//接收服务器信息超时计数
    int CSQ = 0;//
    String IMEI = "";//国际移动设备识别码
	String SIMCCID = "";//sim卡的识别标识.
    String LOCData = "";//LOCData
    String SoftwareVer = "V3.0.0";//软件版本
    String HardwareVer = "V1.0.0";//硬件版本
    char cmd16array[16] = {0};
    const char *cmdarray = cmd16array;
    const unsigned int OUT_NUM_LIST[MAX_OUT_NUM] = {KCZJ1, KCZJ2};



    void Init(void);            //初始化
	void PWR_CON(void);         //
    uint16_t getBattVol(void);  //得到电压
    bool Search_Net(void);      //搜索网络
    bool Access_Net(void);      //连接网络
    void Connect_Station_location_Service(void);//连接基站定位服务
    bool Connect_Server(void);//连接到服务器
    void Client_Check_Connection();//客户端检查连接状态
    void Client_ReceiveCMD();//客户端接收命令
    void ReceiveCMD_Processing(String res);//接收命令处理
    bool SendDataToSever(String string);//发送数据至服务器
    void Send_Heartbeat_Regularly();//定时发送心跳包
};

extern SIM868 Sim868;

#endif
