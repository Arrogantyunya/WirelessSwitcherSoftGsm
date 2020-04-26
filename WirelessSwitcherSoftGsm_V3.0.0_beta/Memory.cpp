/************************************************************************************
 *
 * 代码与注释：刘家辉
 * 日期：2020/4/25
 * 该文件主要功能是依托EEPROM和芯片自带备份寄存器来保存、校验、操作、读取设备相关的信息，
 * 如通用的软件、硬件版本号等。头文件中提供了各个类的公共接口。
 *
 * 如有任何疑问，请发送邮件到： liujiahiu@qq.com
*************************************************************************************/
#include "Memory.h"
#include "User_CRC8.h"
#include "fun_periph.h"
#include <libmaple/bkp.h>
#include <libmaple/iwdg.h>

/*创建EEPROM操作对象*/
EEPROM_Operations EEPROM_Operation;
/*创建软件硬件版本对象*/
Soft_Hard_Vertion Vertion;
/*创建定时模式对象*/
Timing_Mode Decice_Timing_Mode;

struct ALARM Alarm_Array[5];

/*
 @brief     : 设置EEPROM读写引脚
 @para      : 无
 @return    : 无
 */
void EEPROM_Operations::EEPROM_GPIO_Config(void)
{
	pinMode(EP_WP_PIN, OUTPUT);
	digitalWrite(EP_WP_PIN, HIGH);
	I2C_Init();
}

/*
 @brief     : 重置EEPROM的值
 @para      : 无
 @return    : 无
 */
void EEPROM_Operations::EEPROM_Reset(void)
{
	EEPROM_Write_Enable();
	for (size_t i = EEPROM_MIN_ADDR; i < EEPROM_MAX_ADDR; i++)
	{
		AT24CXX_WriteOneByte(i, 0x00);
	}
	EEPROM_Write_Disable();
}

/*
 @brief     : 保存设备模式
 @para      : 无
 @return    : 无
 */
bool Timing_Mode::Save_DeviceMode(unsigned char data)
{
	EEPROM_Write_Enable();
	AT24CXX_WriteOneByte(TIMING_MODE_BASE_ADDR, data);
	EEPROM_Write_Disable();

	if(AT24CXX_ReadOneByte(TIMING_MODE_BASE_ADDR) == data)
	{
		Serial.println("Save DeviceMode Success <Save_DeviceMode>");
		return true;
	}
	else
	{
		Serial.println("Save DeviceMode Fail <Save_DeviceMode>");
		return false;
	}
}

/*
 @brief     : 读取设备模式
 @para      : 无
 @return    : 无
 */
unsigned char Timing_Mode::Read_DeviceMode(void)
{
	unsigned char data = AT24CXX_ReadOneByte(TIMING_MODE_BASE_ADDR);
	return data;
}

/*
 @brief     : 清除设备模式
 @para      : 无
 @return    : 无
 */
bool Timing_Mode::Clean_DeciceMode(void)
{
	EEPROM_Write_Enable();
	AT24CXX_WriteOneByte(TIMING_MODE_BASE_ADDR, 0x00);
	EEPROM_Write_Disable();

	if(AT24CXX_ReadOneByte(TIMING_MODE_BASE_ADDR) == 0x00)
	{
		Serial.println("Clean DeviceMode Success <Save_DeviceMode>");
		return true;
	}
	else
	{
		Serial.println("Clean DeviceMode Fail <Save_DeviceMode>");
		return false;
	}
}