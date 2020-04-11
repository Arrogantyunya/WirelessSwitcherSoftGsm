#ifndef _USER_SIM868_H
#define _USER_SIM868_H

#include <Arduino.h>


// Your GPRS credentials
// Leave empty, if missing user or pass
//普通GSM 2G网络接入点APN为CMNET
//物联网专用接入点APN为CMIOT
const char apn[] = "CMIOT";
const char user[] = "";
const char pass[] = "";

//以下为服务器地址，需要根据实际要连接的服务器地址填写，注意：必须是公网址哦！
//const char server[] = "211.149.155.108";
//int port = 48979;
//const char server[] = "148.70.126.68";
const char server[] = "shhoo.cn";
int port = 6000;

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
    bool GSM_Enter_Net_flag;//进入网络标志
    bool GSM_Search_Net_Flag;//GSM搜索网络标志
    bool LBS_Connect_flag;//LBS连接标志
    bool Sever_Connect_flag;//服务器连接标志位


    void Init(void);         //初始化
	void PWR_CON(void);      //
    uint16_t getBattVoltage(void);//得到电压
    bool Search_Net(void);   //搜索网络
    bool Access_Net(void);   //进入网络
    bool Connect_Station_location_Service(void);//连接基站定位服务
    bool Connect_Server(void);//连接到服务器
};

extern SIM868 Sim868;

#endif
