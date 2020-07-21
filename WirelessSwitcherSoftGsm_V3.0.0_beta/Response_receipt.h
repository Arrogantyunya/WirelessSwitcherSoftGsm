#ifndef _RESPONSE_RECEIPT_H
#define _RESPONSE_RECEIPT_H

#include <Arduino.h>
// #include "fun_periph.h"

enum ReceiptStatus{
    DefaultValue = 0,Set_Param_OK,Set_TimePeriod_overflow,Set_Param_Err
};


class RECEIPT {
private:
    /* data */
public:
	// void Receipt_of_Online(int CSQ,String IMEI,SIMCCID,String LOCData,int MAX_OUT_NUM,String SoftwareVer,String HardwareVer);//初次连接服务器回执
    void Receipt_of_Online();//初次连接服务器回执
    void Receipt_of_Individual_Control(int DOnum, int sta);//单控回执
    void Receipt_of_Batch_Control();//群控回执
    void General_Receipt(int Device_mode,int Device_Status = 0x00);//通用回执
    void Heartbeat_Package_Receipt();//心跳包回执
};

extern RECEIPT Receipt;

#endif