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
    bool General_Control_Mode(String res);//通用控制模式
};

extern CMD cmd;

#endif