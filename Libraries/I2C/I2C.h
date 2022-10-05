#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#define I2C_Freq	100e3

#ifndef I2C_H
#define I2C_H

void I2C1_Init(void);
void I2C_StartCondition(I2C_TypeDef* I2Cx);
void I2C_StopCondition(I2C_TypeDef* I2Cx);
void I2C_SendAddress(I2C_TypeDef* I2Cx, uint8_t Address);
void I2C_SendByte(I2C_TypeDef* I2Cx, uint8_t Byte);
void I2C_SendByteToAddress(I2C_TypeDef* I2Cx, uint8_t Address, uint8_t Byte);
uint8_t I2C_ReadByte(I2C_TypeDef* I2Cx);
uint8_t I2C_ReadLastByte(I2C_TypeDef* I2Cx);

#endif

