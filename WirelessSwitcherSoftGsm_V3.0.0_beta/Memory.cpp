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
char Week[7] = {0};

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

/*
 @brief     : 保存闹钟启用
 @para      : 无
 @return    : 无
 */
bool Timing_Mode::Save_Alarm_Used(unsigned char count)
{
	EEPROM_Write_Enable();
	switch (count)
	{
	case 0: AT24CXX_WriteOneByte(ALARM_0_USED_ADDR, 0x01);break;
	case 1: AT24CXX_WriteOneByte(ALARM_1_USED_ADDR, 0x01);break;
	case 2: AT24CXX_WriteOneByte(ALARM_2_USED_ADDR, 0x01);break;
	case 3: AT24CXX_WriteOneByte(ALARM_3_USED_ADDR, 0x01);break;
	case 4: AT24CXX_WriteOneByte(ALARM_4_USED_ADDR, 0x01);break;
	default:Serial.println("Non-existent ALARM_USED!!!");break;
	}
	EEPROM_Write_Disable();

	switch (count)
	{
	case 0: if(AT24CXX_ReadOneByte(ALARM_0_USED_ADDR) == 0x01){Serial.println("Save ALARM_0_USED Success");return true;}break;
	case 1: if(AT24CXX_ReadOneByte(ALARM_1_USED_ADDR) == 0x01){Serial.println("Save ALARM_1_USED Success");return true;}break;
	case 2: if(AT24CXX_ReadOneByte(ALARM_2_USED_ADDR) == 0x01){Serial.println("Save ALARM_2_USED Success");return true;}break;
	case 3: if(AT24CXX_ReadOneByte(ALARM_3_USED_ADDR) == 0x01){Serial.println("Save ALARM_3_USED Success");return true;}break;
	case 4: if(AT24CXX_ReadOneByte(ALARM_4_USED_ADDR) == 0x01){Serial.println("Save ALARM_4_USED Success");return true;}break;
	default:Serial.println("Non-existent ALARM_USED!!!");break;
	}

	return false;
}

/*
 @brief     : 读取闹钟是否启用
 @para      : 无
 @return    : 无
 */
unsigned char Timing_Mode::Read_Alarm_Used(unsigned char count)
{
	switch (count)
	{
	case 0:return AT24CXX_ReadOneByte(ALARM_0_USED_ADDR); break;
	case 1:return AT24CXX_ReadOneByte(ALARM_1_USED_ADDR); break;
	case 2:return AT24CXX_ReadOneByte(ALARM_2_USED_ADDR); break;
	case 3:return AT24CXX_ReadOneByte(ALARM_3_USED_ADDR); break;
	case 4:return AT24CXX_ReadOneByte(ALARM_4_USED_ADDR); break;
	default:break;
	}
}

/*
 @brief     : 清除所有的闹钟启用
 @para      : 无
 @return    : 无
 */
bool Timing_Mode::Clean_Alarm_Used(void)
{
	EEPROM_Write_Enable();
	for (size_t i = 0; i < 5; i++)
	{
		switch (i)
		{
		case 0: AT24CXX_WriteOneByte(ALARM_0_USED_ADDR, 0x00);break;
		case 1: AT24CXX_WriteOneByte(ALARM_1_USED_ADDR, 0x00);break;
		case 2: AT24CXX_WriteOneByte(ALARM_2_USED_ADDR, 0x00);break;
		case 3: AT24CXX_WriteOneByte(ALARM_3_USED_ADDR, 0x00);break;
		case 4: AT24CXX_WriteOneByte(ALARM_4_USED_ADDR, 0x00);break;
		default:break;
		}
	}
	EEPROM_Write_Disable();
	if( AT24CXX_ReadOneByte(ALARM_0_USED_ADDR) == 0x00 && AT24CXX_ReadOneByte(ALARM_1_USED_ADDR) == 0x00 &&
		AT24CXX_ReadOneByte(ALARM_2_USED_ADDR) == 0x00 && AT24CXX_ReadOneByte(ALARM_3_USED_ADDR) == 0x00 &&
		AT24CXX_ReadOneByte(ALARM_4_USED_ADDR) == 0x00)
	{
		Serial.println("Claen Alarm Used Success");
		return true;
	}
	else
	{
		Serial.println("Claen Alarm Used Fail");
		return false;
	}
	
}