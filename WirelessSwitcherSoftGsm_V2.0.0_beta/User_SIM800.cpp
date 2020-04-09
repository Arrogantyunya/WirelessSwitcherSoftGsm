/************************************************************************************
 *
 * 代码与注释：刘家辉
 * 日期：2020/4/9
 *
 * 一些关于SIM800的函数
 *
 * 如有任何疑问，请发送邮件到： liujiahiu@qq.com
*************************************************************************************/

#include "User_SIM800.h"
#include "fun_periph.h"
#include <libmaple/iwdg.h>
GPRS SIM800;


void GPRS::SIM800_PWR_CON(void)
{
  GPRS_PWRKEY_LOW;
  delay(1500);
  GPRS_PWRKEY_HIGH;
}
