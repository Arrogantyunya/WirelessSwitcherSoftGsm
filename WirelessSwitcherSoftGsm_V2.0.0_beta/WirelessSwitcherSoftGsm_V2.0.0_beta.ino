/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * 
 *
 * TinyGSM Getting Started guide:
 *   https://github.com/ynkady/TinyGSM
 *
 **************************************************************/
/*
 使用注意：
 本测试程序适用SIM８６８ 多功能GSM＋GPS模块，使用前需注意以下事项：
 1、此模块适用于兑悦物联小店的GSMandGPS_MDV1.2及以上版本的模块，其它SIM８６８模块需要对IO进行修改
 https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-16679354584.19.5a9c5712xGAkkw&id=567100266868
 ２、本测试程序适用于Arduino UNO、Mega2560官方版本及其它国内各种改进版本，UNO板也可以参考本店的：
 https://item.taobao.com/item.htm?spm=a1z10.3-c-s.w4002-16618966406.83.339d69c4yPsWMK&id=558178885996
 3、使用本测度程序前需下载TinyGSM库，但原来官方的库里不支持基站定位，我们对这个库进行了更改，并上传到Github,可从以下地址下载，并替换原来的库文件
 https://github.com/ynkady/TinyGSM
 Windows的Arduino库文件地址：C:\Users\kady\Documents\Arduino\libraries
 4、GSM模块硬件连接
 4V---->外部电源供电+（3.4~4.2V,推荐4.0V）
 GND--->外部电源-
 GND--->Arduino UNO GND
 PWRKEY-----> D4
 RX---------> D3
 TX---------> D2   
 特别注意：由于GSM模块要搜索移动网络时瞬间电流非常大，可能达到2A，所以GSM模块不能使用ARDUINO开发板上的电源供电，需要使用外部稳定的电源供电，或使用容量大于1500mAh以上的锂电池供电
 GSM模块供电模块可以从我们的小店找到：
 https://item.taobao.com/item.htm?spm=a1z10.3-c-s.w4002-16618966406.41.339d69c4yPsWMK&id=554761942043
 */

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266
#include <TinyGsmClient.h>
#include <Arduino_JSON.h>
#include <RTClock.h>
#include "User_Clock.h"
#include "fun_periph.h"
#include "User_SIM800.h"

// Your GPRS credentials
// Leave empty, if missing user or pass
//普通GSM 2G网络接入点APN为CMNET
//物联网专用接入点APN为CMIOT
const char apn[] = "CMIOT";
const char user[] = "";
const char pass[] = "";

#define RTC_FUN           true


// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

#if RTC_FUN
RTClock InRtc(RTCSEL_LSE); // initialise RTC
#endif

bool SendDataToSever(String string);
void RTC_Interrupt(void);


//以下为服务器地址，需要根据实际要连接的服务器地址填写，注意：必须是公网址哦！
//const char server[] = "211.149.155.108";
//int port = 48979;
//const char server[] = "148.70.126.68";
const char server[] = "shhoo.cn";
int port = 6000;

bool GSM_Search_Net_Flag;//GSM搜索网络标志
bool LBS_Connect_flag;//LBS连接标志
bool GSM_Enter_Net_flag;//输入网络标志
bool Sever_Connect_flag;//服务器连接标志位
unsigned int TimeSec = 0;//运行超时计数
bool SendHeatBeatFlag = false;//发送心跳包标志位
unsigned int GPRSV = 0;//GPRS的电压
unsigned int Try_Connect_Sever_Num = 0;//尝试连接服务器次数
char cmd16array[16] = {0};
const char *cmdarray = cmd16array;
char Toggle = 0; //状态灯闪灭位翻转
unsigned int ReviceServerTimeoutNum = 0;

const unsigned int OUT_NUM_LIST[MAX_OUT_NUM] = {RELAY_OUT1_PIN, RELAY_OUT2_PIN};

#if RTC_FUN
  UTCTimeStruct RtcTime = {20, 20, 4, 9, 15, 16, 30};
#endif

static const unsigned char SoftWareVer[] = "V1.0.1";

void setup()
{
  // Set console baud rate
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);
  
  Some_Peripheral.Peripheral_GPIO_Pinmode();
  Some_Peripheral.Peripheral_GPIO_Config();

  Serial.begin(115200);
  SerialAT.begin(9600);
  SIM800.SIM800_PWR_CON();
  // Set GSM module baud rate
  delay(3000);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println(F("Initializing modem..."));
  modem.restart();
  //打开GPS电源
  modem.GPS_PWR_CONTROL(ON);
  delay(2000);
  modem.GPS_Set_parameter();
  delay(1000);
  LBS_Connect_flag = false;
  GSM_Search_Net_Flag = false;
  GSM_Enter_Net_flag = false;
  //Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  Sever_Connect_flag = false;
  //初始化定时器2
  Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
  Timer2.setPeriod(1000000); // in microseconds，1S
  Timer2.setCompare1(1);     // overflow might be small
  Timer2.attachCompare1Interrupt(Time2_Handler);

#if RTC_FUN
  //初始化RTC闹钟
  noInterrupts();
  time_t Alarm_Time = 0;
  Alarm_Time = InRtc.getTime();
  InRtc.createAlarm(RTC_Interrupt, Alarm_Time + 60);
  interrupts();
#endif
}

void loop()
{
  String IMEI;
  String SIMCCID;
  int CSQ = 0;
  // Get GPRS Moudle Voltage
  GPRSV = modem.getBattVoltage();
  Serial.print(String("GPRS Moudle Vol:") + GPRSV + "mv");

  if (GSM_Search_Net_Flag == false)
  {
    Serial.print(F("Waiting for network..."));
    if (!modem.waitForNetwork())
    {
      Serial.println(" fail");
      delay(10000);
      GSM_Search_Net_Flag = false;
      return;
    }
    GSM_Search_Net_Flag = true;
    Serial.println("Search GSM Network OK");
  }

  if (GSM_Enter_Net_flag == false && GSM_Search_Net_Flag == true)
  {
    Serial.print(F("Connecting to "));
    Serial.print(apn);
    if (!modem.gprsConnect(apn, user, pass))
    {
      Serial.println(" Fail");
      delay(10000);
      GSM_Enter_Net_flag = false;
      return;
    }

    Serial.println(" Success");
    GSM_Enter_Net_flag = true;
    //send SMS
    //modem.sendSMS("+8613577182976", "this is SIM868");
  }
  //连接基站定位服务连接基站定位服务
  if (LBS_Connect_flag == false)
  {
    if (modem.LBS_Connect() == true)
    {
      LBS_Connect_flag = true;
    }
    else
    {
      Serial.println("LBS Connect Fail!");
      LBS_Connect_flag = false;
    }
  }

  String LOCData;
  if (LBS_Connect_flag == true)
  {
    Serial.println("LBS Connect OK!");
    //0,102.654510,25.064799,550,17/09/14,11:20:30
    LOCData = modem.Get_LOCData();
    Serial.println("LBSData:" + LOCData + "\r\n");
    LBS_Connect_flag = false;
  }

  //String GPSData =modem.GPS_Get_Data();
  //Serial.println("GPS Data:" + GPSData + "\r\n");
  //SerialAT.println("AT+CGNSINF\r\n");
  // delay(2000);

  if (Sever_Connect_flag == false)
  {
    //连接基站定位服务连接基站定位服务
    Serial.print(F("Connecting to Server:"));
    Serial.print(server);
    if (!client.connect(server, port))
    {
      Serial.println(" fail");
      delay(10000);
      Sever_Connect_flag = false;
      Try_Connect_Sever_Num++;
      //连接10次都连接不上服务器，重新入网
      if (Try_Connect_Sever_Num > 10)
      {
        Try_Connect_Sever_Num = 0;
        GSM_Search_Net_Flag = false;
        GSM_Enter_Net_flag = false;
        modem.gprsDisconnect();
        LBS_Connect_flag = false;
      }
      return;
    }
    else
    {
      Serial.println(" OK!");
      Try_Connect_Sever_Num = 0;
      Sever_Connect_flag = true;
      //发送连接信息组服务器
      CSQ = 0;
      CSQ = modem.getSignalQuality();
      Serial.println("\nGSM Enter network OK\r");
      char string[25];
      sprintf(string, "CSQ:%d\r", CSQ);
      Serial.println(string);
      IMEI = modem.getIMEI();
      Serial.println("IMEI:" + IMEI);
      //Serial.print(CSQ,DEC);
      Serial.println("\r");
      SIMCCID = modem.getSimCCID();
      Serial.println("SIMCCID:" + SIMCCID);
      JSONVar myObject;
      myObject["CSQ"] = CSQ;
      myObject["IMEI"] = IMEI;
      myObject["CCID"] = SIMCCID;
      myObject["LBS"] = LOCData;
      Serial.print("myObject.keys() = ");
      Serial.println(myObject.keys());
      // JSON.stringify(myVar) can be used to convert the json var to a String
      String jsonString = JSON.stringify(myObject);
      Serial.print("JSON.stringify(myObject) = ");
      Serial.println(jsonString);
      SendDataToSever(jsonString);
      IMEI.trim();
      SIMCCID.trim();
      LOCData.trim();
      delay(1000);
    }
  }
  // Make a HTTP GET request:
  //AT+OUT:1=ON

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 5000L)
  {
    // Print available data
    if (client.available())
    {
      String res = client.readStringUntil('\n');
      if (res.equals("") == false)
      {
        JSONVar cmdObject = JSON.parse(res);
        // JSON.typeof(jsonVar) can be used to get the type of the var
        if (JSON.typeof(cmdObject) == "undefined")
        {
          Serial.println("Parsing input failed!");
          return;
        }
        Serial.print("JSON.typeof(myObject) = ");
        Serial.println(JSON.typeof(cmdObject)); // prints: object

        // myObject.hasOwnProperty(key) checks if the object contains an entry for key
        if (cmdObject.hasOwnProperty("time"))
        {
          ReviceServerTimeoutNum = 0;
          Serial.print("cmdObject[\"time\"] = ");
          Serial.println(cmdObject["time"]);
          //20200306183358
          const char *DataArray = (const char *)cmdObject["time"];
//String DataString = JSON.stringify(cmdObject["time"]);
#if RTC_FUN
          RtcTime.year = (DataArray[0] - '0') * 1000 + (DataArray[1] - '0') * 100 + (DataArray[2] - '0') * 10 + (DataArray[3] - '0');
          RtcTime.month = (DataArray[4] - '0') * 10 + (DataArray[5] - '0');
          RtcTime.day = (DataArray[6] - '0') * 10 + (DataArray[7] - '0');

          RtcTime.hour = (DataArray[8] - '0') * 10 + (DataArray[9] - '0');
          RtcTime.minutes = (DataArray[10] - '0') * 10 + (DataArray[11] - '0');
          RtcTime.seconds = (DataArray[12] - '0') * 10 + (DataArray[13] - '0');

          UTCTime CurrentSec = osal_ConvertUTCSecs(&RtcTime);
          InRtc.setTime(CurrentSec);
          noInterrupts();
          time_t Alarm_Time = 0;
          Alarm_Time = InRtc.getTime();
          InRtc.createAlarm(RTC_Interrupt, Alarm_Time + 60);
          interrupts();
#endif
        }
        if (cmdObject.hasOwnProperty("DO16Sta"))
        {
          Serial.print("DO16Sta[\"DO16Sta\"] = ");
          //char cmd=cmdObject["DO16Sta"];
          //String CmdString = JSON.stringify(cmdObject["DO16Sta"]);
          //Serial.println(CmdString);
          int i = 0;
          //const char cmdarray[16]={0};
          cmdarray = (const char *)cmdObject["DO16Sta"];
          //CmdString.toCharArray(cmdarray, 16);
          for (i = 0; i < 16; i++)
          {
            if (cmdarray[i] == 0x31)
            {
              //Serial.println(i+"channel Open");
              switch (i)
              {
              case 0:
                RELAY_OUT1_ON;
                break;
              case 1:
                RELAY_OUT2_ON;
                break;
              default:
                break;
              }
            }
            else if (cmdarray[i] == 0x30)
            {
              //Serial.println(i+"channel close");
              switch (i)
              {
              case 0:
                RELAY_OUT1_OFF;
                break;
              case 1:
                RELAY_OUT2_OFF;
                break;
              default:
                break;
              }
            }
            else
            {
              Serial.println("error!");
              //Serial.println(cmdarray[i]);
            }
            delay(10);
          }
          memset(cmd16array, '0', sizeof(cmd16array));
          //读输出IO的状态
          for (int i = 0; i < MAX_OUT_NUM; i++)
          {
            if (digitalRead(OUT_NUM_LIST[i]) == LOW)
            {
              cmd16array[i] = '1';
            }
            else if (digitalRead(OUT_NUM_LIST[i]) == HIGH)
            {
              cmd16array[i] = '0';
            }
          }

          //发送应答
          JSONVar AckObject;
          AckObject["IMEI"] = IMEI;
          AckObject["DO16Sta"] = cmd16array;
          Serial.println(IMEI);
          Serial.println(cmdarray);
          Serial.print("AckObject.keys() = ");
          Serial.println(AckObject.keys());
          // JSON.stringify(myVar) can be used to convert the json var to a String
          String jsonString1 = JSON.stringify(AckObject);
          Serial.print("JSON.stringify(myObject) = ");
          Serial.println(jsonString1);
          // client.print(jsonString1);
          char *sendBuf;
          int datalength = jsonString1.length() + 1;
          sendBuf = (char *)malloc(datalength);
          jsonString1.toCharArray(sendBuf, datalength);
          client.write((unsigned char *)sendBuf, datalength);
          free(sendBuf);
        }
        if (cmdObject.hasOwnProperty("DONum"))
        {
          int ChNum = (int)cmdObject["DONum"];
          if (cmdObject.hasOwnProperty("sta"))
          {
            int sta = (int)cmdObject["sta"];
            if (sta == 1)
            {
              switch (ChNum)
              {
              case 255:
                RELAY_OUT1_ON;
                RELAY_OUT2_ON;
                break;
              case 1:
                RELAY_OUT1_ON;
                break;
              case 2:
                RELAY_OUT2_ON;
                break;
              default:
                break;
              }
            }
            else if (sta == 0)
            {
              switch (ChNum)
              {
              case 255:
                RELAY_OUT1_OFF;
                RELAY_OUT2_OFF;
                break;
              case 1:
                RELAY_OUT1_OFF;
                break;
              case 2:
                RELAY_OUT2_OFF;
                break;
              default:
                break;
              }
            }

            //发送应答
            JSONVar AckObject;
            AckObject["IMEI"] = IMEI;
            AckObject["DONum"] = ChNum;
            AckObject["sta"] = sta;
            Serial.print("AckObject.keys() = ");
            Serial.println(AckObject.keys());
            // JSON.stringify(myVar) can be used to convert the json var to a String
            String jsonString1 = JSON.stringify(AckObject);
            Serial.print("JSON.stringify(myObject) = ");
            Serial.println(jsonString1);
            //client.print(jsonString1);
            SendDataToSever(jsonString1);
          }
        }
      }
      res.trim();
    }

    //定时发送心跳包
    if (SendHeatBeatFlag == true && Sever_Connect_flag)
    {
      SendHeatBeatFlag = false;
      JSONVar AckObject;
      memset(cmd16array, 0, sizeof(cmd16array));
      AckObject["IMEI"] = IMEI;
      AckObject["DO16Sta"] = cmd16array;
      Serial.print("AckObject.keys() = ");
      Serial.println(AckObject.keys());
      // JSON.stringify(myVar) can be used to convert the json var to a String
      String jsonString1 = JSON.stringify(AckObject);
      Serial.print("JSON.stringify(myObject) = ");
      Serial.println(jsonString1);
      //client.print(jsonString1);
      if (client.connected())
      {
        SendDataToSever(jsonString1);
      }
      ReviceServerTimeoutNum++;
      Serial.print("ReviceServerTimeoutNum:");
      Serial.println(ReviceServerTimeoutNum);
      //如果接收服务器数据次数超时
      if (ReviceServerTimeoutNum >= 10)
      {
        ReviceServerTimeoutNum = 0;
        if (Sever_Connect_flag == true)
        {
          Sever_Connect_flag = false;
          client.stop();
        }
      }
    }

    timeout = millis();
  }

  if (!client.connected())
  {
    if (Sever_Connect_flag == true)
    {
      Sever_Connect_flag = false;
      client.stop();
    }
  }
}

bool SendDataToSever(String string)
{
  char *sendBuf;
  int datalength = string.length() + 1;
  sendBuf = (char *)malloc(datalength);
  string.toCharArray(sendBuf, datalength);
  client.write((unsigned char *)sendBuf, datalength);
  free(sendBuf);
  return true;
}
/*
 *brief   : 定时器2中断函数
 *para    : 无
 *return  : 无
*/
void Time2_Handler(void)
{
  //Toggle ^= 1;
  //digitalWrite(RELAY_OUT1_PIN, Toggle); //状态灯闪烁
  TimeSec++;
  //如果运行超时，复位
  if (TimeSec >= 30)
  {
    TimeSec = 0;
    SendHeatBeatFlag = true;
  }
}

#if RTC_FUN
void RTC_Interrupt(void)
{
  rtc_detach_interrupt(RTC_ALARM_SPECIFIC_INTERRUPT);
  Serial.println("RTC alarm interrupt");
}
#endif
