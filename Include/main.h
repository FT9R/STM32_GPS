#ifndef MAIN_H
#define MAIN_H

#include "stm32f10x.h"
#include "Sys_Init.h"
#include "Delay.h"
#include "ssd1306.h"
#include "uart.h"
#include "GPS.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SystemCoreClock
#define LED_ON									CLEAR_BIT(GPIOC->ODR, 1 << 13)
#define LED_OFF									SET_BIT(GPIOC->ODR, 1 << 13)
#define TIME_CORRECTION_HOUR		3
#define INVOLVE_DISPLAY

void IO_Init(void);
void Start_Blink(void);

#endif