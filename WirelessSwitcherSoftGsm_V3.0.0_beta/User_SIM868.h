#ifndef _USER_SIM868_H
#define _USER_SIM868_H

#include <Arduino.h>
#include "fun_periph.h"

#define RTC_FUN 	true
#define TEST        true

#define CMIOT       true//移动2G物联卡

#define CMWAP       false//移动2G移动互联网
#define CMNET       false//移动2G连接互联网()
#define UNIWAP      false//联通2G移动互联网
#define UNINET      false//联通2G连接互联网()
#define CTWAP       false//电信2G移动互联网
#define CTNET       false//电信2G连接互联网()


// Your GPRS credentials
// Leave empty, if missing user or pass
//普通GSM 2G网络接入点APN为CMNET
//物联网专用接入点APN为CMIOT
#if CMIOT
const char apn[] = "CMIOT";
const char user[] = "";
const char pass[] = "";
#elif CMNET
const char apn[] = "CMNET";
const char user[] = "";
const char pass[] = "";
#elif UNINET
const char apn[] = "UNINET";
const char user[] = "";
const char pass[] = "";
#elif CTNET
const char apn[] = "CTNET";
const char user[] = "";
const char pass[] = "";
#else
#endif


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


    void Init(void);            //初始化
	void PWR_CON(void);         //
    uint16_t getBattVol(void);  //得到电压
    bool Search_Net(void);      //搜索网络
    bool Access_Net(void);      //连接网络
    void Connect_Station_location_Service(void);//连接基站定位服务
    bool Connect_Server(void);//连接到服务器
    void Client_Check_Connection();//客户端检查连接状态
    void Client_ReceiveCMD();//客户端接收命令
    void ReceiveCMD_Analysis(String res);//接收命令处理
    bool SendDataToSever(String string);//发送数据至服务器
    void Send_Heartbeat_Regularly();//定时发送心跳包
};

extern SIM868 Sim868;

extern String IMEI;//国际移动设备识别码
extern String SIMCCID;//sim卡的识别标识.
extern String LOCData;//LOCData
extern String SoftwareVer;//软件版本
extern String HardwareVer;//硬件版本
extern int CSQ;//信号质量
extern char cmd16array[16];
extern const char *cmdarray;
extern const unsigned int OUT_NUM_LIST[MAX_OUT_NUM];
extern int Decice_Mode;//设备工作模式
extern int Decice_States;//设备工作状态

#endif
