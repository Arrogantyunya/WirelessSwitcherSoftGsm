#ifndef _Memory_H
#define _Memory_H

#include "AT24CXX.h"
#include <libmaple/bkp.h>
#include <Arduino.h>

#define EP_WP_PIN                               PB5

/*使用EEPROM储存芯片的宏定义地址*/
/*EEPROM硬件特性*/
#define EEPROM_MIN_ADDR                         0
#define EEPROM_MAX_ADDR                         255

/* 定时模式相关的保存地址 */
#define TIMING_MODE_BASE_ADDR					1


/*使用芯片自带备份寄存器的宏定义地址*/
/*上一次开度值保存地址（0 - 100）*/
#define BKP_MOTOR_LAST_OPENING_ADDR             1
/*上一次开度值CRC8保存地址*/
#define BKP_MOTOR_LAST_OPENING_CRC_ADDR         2
/*本次开度值保存地址（0 - 100）*/
#define BKP_MOTOR_RECENT_OPENING_ADDR           3
/*本次开度值CRC8保存地址*/
#define BKP_MOTOR_RECENT_OPENING_CRC_ADDR       4
/*实时开度值保存地址*/
#define BKP_MOTOR_REALTIME_OPENING_ADDR         5 
/*实时开度值CRC8保存地址*/
#define BKP_MOTOR_REALTIME_OPENING_CRC_ADDR     6   

extern String comdata;

struct ALARM
{
    char Begin[8];//
    char End[8];//
    char DoUsed[16];//
};

extern struct ALARM Alarm_Array[5];

/*
 @brief     : 上拉该引脚，禁止EEPROM写操作
 @para      : 无
 @return    : 无
 */
inline void EEPROM_Write_Disable(void)
{
	digitalWrite(EP_WP_PIN, HIGH);
}

/*
 @brief     : 下拉该引脚，允许EEPROM写操作
 @para      : 无
 @return    : 无
 */
inline void EEPROM_Write_Enable(void)
{
	digitalWrite(EP_WP_PIN, LOW);
}

class EEPROM_Operations : protected AT24Cxx {
public:
	void EEPROM_GPIO_Config(void);
	void EEPROM_Reset(void);//将EP的值重置
};

class Soft_Hard_Vertion : public EEPROM_Operations {
public:
	
};

class Timing_Mode : public EEPROM_Operations{
private:
	/* data */
public:
	bool Save_DeviceMode(unsigned char data);//保存设备模式
	unsigned char Read_DeviceMode(void);//读取设备模式
	bool Clean_DeciceMode(void);//清除设备模式
};

extern EEPROM_Operations EEPROM_Operation;
extern Soft_Hard_Vertion Vertion;
extern Timing_Mode Decice_Timing_Mode;

#endif
