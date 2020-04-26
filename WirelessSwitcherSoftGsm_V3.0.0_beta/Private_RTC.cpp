#include "Private_RTC.h"
#include <Arduino.h>
// #include "Memory.h"
#include "public.h"
// #include "Command_Analysis.h"

/*RCT object*/
RTClock Date(RTCSEL_LSE);
UTCTimeStruct RTCTime;

date Private_RTC;

//bool LowBalFlag = false;
unsigned char LowBalFlag = 0;
bool gRTCTime_arrive_Flag = false;//RTC时间到达的标志位
unsigned char const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表    

void date::Update_RTC(unsigned char *buffer)
{
#if USE_HEX
	RTCTime.year = Type_Conv.Dec_To_Hex(buffer[0]) * 100 + Type_Conv.Dec_To_Hex(buffer[1]);
	RTCTime.month = Type_Conv.Dec_To_Hex(buffer[2]);
	RTCTime.day = Type_Conv.Dec_To_Hex(buffer[3]);
	RTCTime.hour = Type_Conv.Dec_To_Hex(buffer[4]);
	RTCTime.minutes = Type_Conv.Dec_To_Hex(buffer[5]);
	RTCTime.seconds = Type_Conv.Dec_To_Hex(buffer[6]);
#elif USE_DEC
	RTCTime.year = buffer[0] * 100 + buffer[1];
	RTCTime.month = buffer[2];
	RTCTime.day = buffer[3];
	RTCTime.hour = buffer[4];
	RTCTime.minutes = buffer[5];
	RTCTime.seconds = buffer[6];
#endif

	Serial.print("year: "); Serial.println(RTCTime.year);
	Serial.print("month: "); Serial.println(RTCTime.month);
	Serial.print("day: "); Serial.println(RTCTime.day);
	Serial.print("hour: "); Serial.println(RTCTime.hour);
	Serial.print("minutes: "); Serial.println(RTCTime.minutes);
	Serial.print("seconds: "); Serial.println(RTCTime.seconds);

	bkp_enable_writes();
	delay(100);
	UTCTime CurrentSec = osal_ConvertUTCSecs(&RTCTime);
	Date.setTime(CurrentSec);
	delay(100);
	bkp_disable_writes();

	CurrentSec = Date.getTime();
	osal_ConvertUTCTime(&RTCTime, CurrentSec);

#if USE_HEX
	buffer[0] = Type_Conv.Hex_To_Dec(RTCTime.year / 100);
	buffer[1] = Type_Conv.Hex_To_Dec(RTCTime.year % 1000);
	buffer[2] = Type_Conv.Hex_To_Dec(RTCTime.month);
	buffer[3] = Type_Conv.Hex_To_Dec(RTCTime.day);
	buffer[4] = Type_Conv.Hex_To_Dec(RTCTime.hour);
	buffer[5] = Type_Conv.Hex_To_Dec(RTCTime.minutes);
	buffer[6] = Type_Conv.Hex_To_Dec(RTCTime.seconds);

	for (unsigned char i = 0; i < 7; i++) {
		Serial.print(buffer[i], HEX);
		Serial.print(" ");
	}
	Serial.print("(HEX)");
	Serial.println();
#elif USE_DEC
	buffer[0] = RTCTime.year / 100;
	buffer[1] = RTCTime.year % 1000;
	buffer[2] = RTCTime.month;
	buffer[3] = RTCTime.day;
	buffer[4] = RTCTime.hour;
	buffer[5] = RTCTime.minutes;
	buffer[6] = RTCTime.seconds;

	for (unsigned char i = 0; i < 7; i++) {
		Serial.print(buffer[i], DEC);
		Serial.print(" ");
	}
	Serial.print("(DEC)");
	Serial.println();
#endif

	Serial.println("Update RTC SUCCESS... <Update_RTC>");
	Serial.println("");
}

void date::Get_RTC(unsigned char *buffer)
{
	UTCTime CurrentSec = 0;
	CurrentSec = Date.getTime();
	osal_ConvertUTCTime(&RTCTime, CurrentSec);
#if USE_HEX
	buffer[0] = Type_Conv.Hex_To_Dec(RTCTime.year / 100);
	buffer[1] = Type_Conv.Hex_To_Dec(RTCTime.year % 1000);
	buffer[2] = Type_Conv.Hex_To_Dec(RTCTime.month);
	buffer[3] = Type_Conv.Hex_To_Dec(RTCTime.day);
	buffer[4] = Type_Conv.Hex_To_Dec(RTCTime.hour);
	buffer[5] = Type_Conv.Hex_To_Dec(RTCTime.minutes);
	buffer[6] = Type_Conv.Hex_To_Dec(RTCTime.seconds);

	for (unsigned char i = 0; i < 7; i++) {
		Serial.print(buffer[i], HEX);
		Serial.print(" ");
	}
	Serial.print("(HEX)");
	Serial.println();
#elif USE_DEC
	buffer[0] = RTCTime.year / 100;
	buffer[1] = RTCTime.year % 1000;
	buffer[2] = RTCTime.month;
	buffer[3] = RTCTime.day;
	buffer[4] = RTCTime.hour;
	buffer[5] = RTCTime.minutes;
	buffer[6] = RTCTime.seconds;

	for (unsigned char i = 0; i < 7; i++) {
		Serial.print(buffer[i], DEC);
		Serial.print(" ");
	}
	Serial.print("(DEC)");
	Serial.println();
#endif
}


/*
 *brief   : RTC alarm interrupt wake-up device
 *para    : None
 *return  : None
 */
void RTC_Interrupt(void)
{
	//Keep this as short as possible. Possibly avoid using function calls
	rtc_detach_interrupt(RTC_ALARM_SPECIFIC_INTERRUPT);//RTC报警特定中断
	gRTCTime_arrive_Flag = true;
	//nvic_sys_reset();//系统重启
}

/*
 *brief   : Initalize setting alarm clock
 *para    : None
 *return  : None
 */
void date::Init_Set_Alarm(void)
{
	bkp_enable_writes();
	delay(100);
	time_t Alarm_Time = 0;
	Alarm_Time = Date.getTime();
	Alarm_Time += 180;
	Date.createAlarm(RTC_Interrupt, Alarm_Time);
	delay(100);
	bkp_disable_writes();
}

/*
 *brief   : Read alarm clock number from EEPROM and set alarm clock
 *para    : None
 *return  : None
 */
void date::Set_Alarm(void)
{
	UTCTime CurrentSec = osal_ConvertUTCSecs(&RTCTime);

	bkp_enable_writes();
	delay(10);
	Date.setTime(CurrentSec);

	unsigned long int alarm = Date.getTime(); //Get current time.

	alarm += 0;//设置闹钟时间
	// Serial.println(String("设置的循环间隔时间Cyclic_interval = ") + Cyclic_interval);

	Date.createAlarm(RTC_Interrupt, alarm);
	delay(10);
	bkp_disable_writes();
}

/*
 *brief   : Read alarm clock number from EEPROM and set alarm clock
 *para    : None
 *return  : None
 */
void date::Set_onehour_Alarm(void)
{
	UTCTime CurrentSec = osal_ConvertUTCSecs(&RTCTime);

	bkp_enable_writes();
	delay(10);
	Date.setTime(CurrentSec);

	unsigned long int alarm = Date.getTime(); //Get current time.

	alarm += 3600;
	Serial.println(String("电池电压低于2.8V!!!直接进入休眠，设置的间隔休眠时间为1小时"));
	Date.createAlarm(RTC_Interrupt, alarm);
	delay(10);
	bkp_disable_writes();
}


//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//year,month,day：公历年月日 
//返回值：星期号(1~7,代表周1~周日) 
unsigned char date::RTC_Get_Week(unsigned short int year,unsigned char month,unsigned char day)
{ //按年月日计算星期(只允许1901-2099年)//已由RTC_Get调用  
	Serial.print(String(year) + "." + month + "." + day + "<RTC_Get_Week>");
	unsigned short int temp2;
	unsigned char yearH,yearL;
	yearH=year/100;     
	yearL=year%100;
	// 如果为21世纪,年份数加100 
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的 
	temp2=yearL+yearL/4;
	temp2=temp2%7;
	temp2=temp2 + day + table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7); //返回星期值
}

/* 
********************************************************************************************************* 
*    函 数 名: bsp_CalcWeek 
*    功能说明: 根据日期计算星期几 
*    形    参: _year _mon _day  年月日  (年是2字节整数，月和日是字节整数） 
*    返 回 值: 周几 （1-7） 7表示周日 
********************************************************************************************************* 
*/  
int date::WeekDayCount(int _year, int _mon, int _day)  
{  
    /*  
    蔡勒（Zeller）公式  
        历史上的某一天是星期几？未来的某一天是星期几？关于这个问题，有很多计算公式（两个通用计算公式和 
    一些分段计算公式），其中最著名的是蔡勒（Zeller）公式。 
        即w=y+[y/4]+[c/4]-2c+[26(m+1)/10]+d-1  
         
        公式中的符号含义如下， 
         w：星期； 
         c：年的高2位，即世纪-1 
         y：年（两位数）； 
         m：月（m大于等于3，小于等于14，即在蔡勒公式中，某年的1、2月要看作上一年的13、14月来计算， 
            比如2003年1月1日要看作2002年的13月1日来计算）； 
         d：日； 
         [ ]代表取整，即只要整数部分。 
         
        算出来的W除以7，余数是几就是星期几。如果余数是0，则为星期日。  
        如果结果是负数，负数求余数则需要特殊处理： 
            负数不能按习惯的余数的概念求余数，只能按数论中的余数的定义求余。为了方便  
        计算，我们可以给它加上一个7的整数倍，使它变为一个正数，然后再求余数     
         
        以2049年10月1日（100周年国庆）为例，用蔡勒（Zeller）公式进行计算，过程如下：  
        蔡勒（Zeller）公式：w=y+[y/4]+[c/4]-2c+[26(m+1)/10]+d-1  
        =49+[49/4]+[20/4]-2×20+[26× (10+1)/10]+1-1  
        =49+[12.25]+5-40+[28.6]  
        =49+12+5-40+28  
        =54 (除以7余5)  
        即2049年10月1日（100周年国庆）是星期5。 
    */  
    int y, c, m, d;  
    int16_t w;  
	Serial.println(String(_year) + "年" + _mon + "月" + _day + "日<xingqi>");
	
  
    if (_mon >= 3)  
    {  
        m = _mon;          
        y = _year % 100;  
        c = _year / 100;  
        d = _day;  
    }  
    else    /* 某年的1、2月要看作上一年的13、14月来计算 */  
    {  
        m = _mon + 12;  
        y = (_year - 1) % 100;  
        c = (_year - 1) / 100;  
        d = _day;  
    }  
  
    w = y + y / 4 +  c / 4 - 2 * c + ((int)26*(m+1))/10 + d - 1;      
    if (w == 0)  
    {  
        w = 7;    /* 表示周日 */  
    }  
    else if (w < 0)    /* 如果w是负数，则计算余数方式不同 */  
    {  
        w = 7 - (-w) % 7;  
    }  
    else  
    {  
        w = w % 7;  
    }  
    return w;  
}

int date::Get_Week(int Year,int Month ,int Day)
{
	//计算当日是星期几（公式从网上查得）

    // int yearCal = dateTime.year();
    // int monthCal = dateTime.month();
    // int dayCal = dateTime.day();
	int yearCal = Year;
    int monthCal = Month;
    int dayCal = Day;
    int weekCal = 0;
	Serial.println(String(yearCal) + "年" + monthCal + "月" + dayCal + "日<Get_Week>");
    
    if (monthCal == 1) monthCal = 13;
    if (monthCal == 2) monthCal = 14;
    //计算
    weekCal = (dayCal + 2*monthCal + 3*(monthCal+1)/5 + yearCal + yearCal/4 - yearCal/100 + yearCal/400) % 7 + 1;

	return weekCal;
}