#ifndef _FUN_PERIPH_H
#define _FUN_PERIPH_H

#include <Arduino.h>

//Selset Dev Board Type
#define UNO 1
#define MEGA2560 2
#define BOARD_TYPE STM32

// Use Hardware Serial on Mega, Leonardo, Micro
#if BOARD_TYPE == MEGA2560
#define SerialAT Serial3
#define GPRS_PWRKEY_PIN 4
#elif BOARD_TYPE == UNO
#define U_RX 2
#define U_TX 3
#define GPRS_PWRKEY_PIN 4
#include "SoftwareSerial.h"
SoftwareSerial SerialAT(U_RX, U_TX);
#elif BOARD_TYPE == STM32
#define SerialAT          Serial2
#define GPRS_PWRKEY_PIN   PA15
#define WLBG_PWR_PIN      PA5
#define WLBG_PWR_ON       digitalWrite(WLBG_PWR_PIN, HIGH)
#define WLBG_PWR_OFF      digitalWrite(WLBG_PWR_PIN, LOW)
#define RELAY_OUT1_PIN    PA3
#define RELAY_OUT2_PIN    PA4
#define RELAY_OUT1_ON     digitalWrite(RELAY_OUT1_PIN, LOW)
#define RELAY_OUT1_OFF    digitalWrite(RELAY_OUT1_PIN, HIGH)
#define RELAY_OUT2_ON     digitalWrite(RELAY_OUT2_PIN, LOW)
#define RELAY_OUT2_OFF    digitalWrite(RELAY_OUT2_PIN, HIGH)
#else
#define U_RX 2
#define U_TX 3
#define GPRS_PWRKEY_PIN 4
#include "SoftwareSerial.h"
SoftwareSerial SerialAT(U_RX, U_TX);
#endif

//根据硬件上的连接来更改
#define GPRS_PWRKEY_HIGH    digitalWrite(GPRS_PWRKEY_PIN, HIGH)
#define GPRS_PWRKEY_LOW     digitalWrite(GPRS_PWRKEY_PIN, LOW)

#define MAX_OUT_NUM       2

class Some_Peripherals {
public:
	/*Configurate some functional pins*/
	void Peripheral_GPIO_Pinmode(void);
	/*Configurate some functional pins*/
	void Peripheral_GPIO_Config(void);
	// void LED_Display(LED which_led, unsigned char freq);
	void Stop_LED(void);
	void Start_LED(void);
	// bool Peripheral_GPIO_Config_Init(void);//外设引脚初始化。
}; 

void LED_Interrupt(void);

/*Create peripheral object*/
extern Some_Peripherals Some_Peripheral;

#endif