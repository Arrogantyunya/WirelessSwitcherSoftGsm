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
#define TIMING_MODE_BASE_ADDR					1//存储的定时模式
#define ALARM_0_USED_ADDR						2//闹钟0是否启用的地址
#define ALARM_1_USED_ADDR						3//闹钟1是否启用的地址
#define ALARM_2_USED_ADDR						4//闹钟2是否启用的地址
#define ALARM_3_USED_ADDR						5//闹钟3是否启用的地址
#define ALARM_4_USED_ADDR						6//闹钟4是否启用的地址
#define ALARM_BEGIN_0_BASE_ADDR					7
#define ALARM_END_0_BASE_ADDR					15
#define ALARM_DOUSED_0_BASE_ADDR				23//闹钟0的存放数组地址
#define ALARM_BEGIN_1_BASE_ADDR					39
#define ALARM_END_1_BASE_ADDR					47
#define ALARM_DOUSED_1_BASE_ADDR				55//闹钟1的存放数组地址
#define ALARM_BEGIN_2_BASE_ADDR					71
#define ALARM_END_2_BASE_ADDR					79
#define ALARM_DOUSED_2_BASE_ADDR				87//闹钟2的存放数组地址
#define ALARM_BEGIN_3_BASE_ADDR					103
#define ALARM_END_3_BASE_ADDR					111
#define ALARM_DOUSED_3_BASE_ADDR				119//闹钟3的存放数组地址
#define ALARM_BEGIN_4_BASE_ADDR					135
#define ALARM_END_4_BASE_ADDR					143
#define ALARM_DOUSED_4_BASE_ADDR				151//闹钟4的存放数组地址
#define Mode1_interval_ADDR						167//Mode1的interval存储地址
#define Mode1_RetryCnt_ADDR						168//Mode1的RetryCnt存储地址
#define Mode2_WEEK_BASE_ADDR					169//Mode2的week数组存储地址
#define Mode2_RetryCnt_ADDR						176//Mode2的RetryCnt存储地址

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
extern char Week[7];
extern unsigned char Mode1_interval;//
extern unsigned char Mode1_RetryCnt;//
extern unsigned char Mode2_RetryCnt;//

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

	bool Save_Alarm_Used(unsigned char count);//保存闹钟启用
	unsigned char Read_Alarm_Used(unsigned char count);//读取闹钟是否启用 
	bool Clean_Alarm_Used(void);//清除所有的闹钟启用

	bool Save_Mode1_interval(unsigned char data);//保存Mode1(通用控制模式)的interval
	unsigned char Read_Mode1_interval(void);//读取Mode1(通用控制模式)的interval
	bool Clean_Mode1_interval(void);//清除Mode1(通用控制模式)的interval

	bool Save_Mode1_RetryCnt(unsigned char data);//保存Mode1(通用控制模式)的RetryCnt
	unsigned char Read_Mode1_RetryCnt(void);//读取Mode1(通用控制模式)的RetryCnt
	bool Clean_Mode1_RetryCnt(void);//清除Mode1(通用控制模式)的RetryCnt

	bool Save_Mode2_Week(unsigned char* data);//保存Mode2(按周控制模式)的week数组
	unsigned char* Read_Mode2_Week(void);//读取Mode2(按周控制模式)的week数组
	bool Clean_Mode2_Week(void);//清除Mode2(按周控制模式)的week数组

	bool Save_Mode2_RetryCnt(unsigned char data);//保存Mode2(按周控制模式)的RetryCnt
	unsigned char Read_Mode2_RetryCnt(void);//读取Mode2(按周控制模式)的RetryCnt
	bool Clean_Mode2_RetryCnt(void);//清除Mode2(按周控制模式)的RetryCnt
};


extern EEPROM_Operations EEPROM_Operation;
extern Soft_Hard_Vertion Vertion;
extern Timing_Mode Decice_Timing_Mode;

#endif
