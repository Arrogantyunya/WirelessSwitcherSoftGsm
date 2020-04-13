/************************************************************************************
 *
 * 代码与注释：刘家辉
 * 日期：2020/4/9
 *
 * 配置STM32F103的通用定时器。
 * 
 * 在STM32F103xx增强型产品中，内置了多达3个可同步运行的标准定时器（TIM2、TIM3、TIM4）。
 * 每个定时器都有一个16位的自动加载递加/递减计数器，一个16位的预分频器和4个独立通道
 * 每个通道都可用于输入捕获、输入比较、PWM和单脉冲模式输出。
 * 在大的封装配置中可提供多12个输入捕获，输出比较或PWM通道。
 * 
 * 它们还能通过定时器链接功能与高级控制定时器共同工作，提供同步或事件链接功能。在调试模式下，计数器可被冻结。
 * 
 * 如有任何疑问，请发送邮件到： liujiahiu@qq.com
*************************************************************************************/
#include "Private_Timer.h"
#include "fun_periph.h"
// #include "Security.h"
// #include "receipt.h"
// #include "Memory.h"
// #include "Set_coil.h"
#include <Arduino.h>



/*Timer timing time*/
#define TIMER_NUM             1000000L * 1 //1S
#define CHECK_TIMER_NUM       1000000L

volatile static unsigned int gTimer2Num;
volatile static unsigned int gTimer3Num;
volatile static unsigned int gTimer4Num;

bool SendHeatBeatFlag = false;			//发送心跳包标志位



/*
 @brief   : 定时器2初始化
 @param   : 无
 @return  : 无
 */
void Timer2_Init(void)
{
	Timer2.setPeriod(TIMER_NUM); //设置周期 in microseconds，1S
	Timer2.attachCompare1Interrupt(Timer2_Interrupt);
	Timer2.setCount(0);
	Timer2.pause();//暂停定时器
}

/*
 @brief   : 定时器3初始化
 @param   : 无
 @return  : 无
 */
void Timer3_Init(void)
{
	Timer3.setPeriod(TIMER_NUM); //设置周期 in microseconds，1S
	Timer3.attachCompare1Interrupt(Timer3_Interrupt);
	Timer3.setCount(0);
	Timer3.pause();//暂停定时器
}

/*
 @brief   : 定时器4初始化
 @param   : 无
 @return  : 无
 */
void Timer4_Init(void)
{
	Timer4.setPeriod(TIMER_NUM); //设置周期 in microseconds，1S
	Timer4.attachCompare1Interrupt(Timer4_Interrupt);
	Timer4.setCount(0);
	Timer4.pause();//暂停定时器
}

/*
 @brief   : 开始定时器2计时
 @param   : 无
 @return  : 无
 */
void Start_Timer2(void)
{
	Timer2.resume();//重新开始
	Timer2.setCount(0);
}

/*
 @brief   : 开始定时器3计时
 @param   : 无
 @return  : 无
 */
void Start_Timer3(void)
{
	Timer3.resume();//重新开始
	Timer3.setCount(0);
}

/*
 @brief   : 开始定时器4计时
 @param   : 无
 @return  : 无
 */
void Start_Timer4(void)
{
	Timer4.resume();//重新开始
	Timer4.setCount(0);
}

/*
 @brief   : 停止定时器2计时
 @param   : 无
 @return  : 无
 */
void Stop_Timer2(void)
{
	Timer2.pause();//暂停定时器
}

/*
 @brief   : 停止定时器3计时
 @param   : 无
 @return  : 无
 */
void Stop_Timer3(void)
{
	Timer3.pause();//暂停定时器
}

/*
 @brief   : 停止定时器4计时
 @param   : 无
 @return  : 无
 */
void Stop_Timer4(void)
{
	Timer4.pause();//暂停定时器
}



/*
 @brief   : 定时器2计时中断处理函数
 @param   : 无
 @return  : 无
 */
void Timer2_Interrupt(void)
{
	gTimer2Num++;
	// Serial.println(String("gTimer2Num = ") + gTimer2Num);
	//如果运行超时，复位
	if (gTimer2Num >= 30)
	{
		gTimer2Num = 0;
		SendHeatBeatFlag = true;
	}
}

/*
 @brief   : 定时器3计时中断处理函数
 @param   : 无
 @return  : 无
 */
void Timer3_Interrupt(void)
{
	gTimer3Num++;
	Serial.println(String("gTimer3Num = ") + gTimer3Num);
}

/*
 @brief   : 灌溉定时器4计时中断处理函数
 @param   : 无
 @return  : 无
 */
void Timer4_Interrupt(void)
{
	gTimer4Num++;
	Serial.println(String("gTimer4Num = ") + gTimer4Num);
}


