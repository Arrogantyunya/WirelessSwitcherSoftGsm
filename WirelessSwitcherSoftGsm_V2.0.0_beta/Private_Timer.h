#ifndef _PRIVATE_TIMER_H
#define _PRIVATE_TIMER_H

void Timer2_Init(void);
void Timer3_Init(void);
void Timer4_Init(void);

void Start_Timer2(void);
void Start_Timer3(void);
void Start_Timer4(void);

void Stop_Timer2(void);
void Stop_Timer3(void);
void Stop_Timer4(void);

void Timer2_Interrupt(void);
void Timer3_Interrupt(void);
void Timer4_Interrupt(void);


#endif