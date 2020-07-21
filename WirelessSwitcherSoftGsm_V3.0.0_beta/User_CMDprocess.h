#ifndef _USER_CMDPROCESS_H
#define _USER_CMDPROCESS_H

#include <Arduino.h>
#include "fun_periph.h"

#define General_Control_Mode    0x01
#define Week_Control_Mode       0x02
#define Stop_Control_Mode       0x00       


class CMD {
private:
    /* data */
public:
	void Save_Rtc(const char* rtc);//保存RTC
    void Individual_Control(int Which_DO,int Sta);//单独控制
    void Batch_Control(const char* Cmd_data);//批量控制
    bool General_Control_Mode_Save(String res);//通用控制模式
    bool Week_Control_Mode_Save(String res);//按周控制模式
    bool Stop_Control_Mode_Save(String res);//停止模式
};

extern CMD cmd;

#endif