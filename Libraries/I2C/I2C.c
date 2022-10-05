#include "I2C.h"


I2C_InitTypeDef I2C1_InitStruct;


void I2C1_Init(void)
{
	I2C_DeInit(I2C1);
	I2C_Cmd(I2C1, DISABLE);
	// RCC
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);	// I2C clock EN
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;	// port B clock enabled
	// Pin config
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
	/*PB6 - SCL*/
	GPIOB->CRL |= GPIO_CRL_MODE6;			// Output mode, max speed 50 MHz
	MODIFY_REG(GPIOB->CRL,GPIO_CRL_CNF6, GPIO_CRL_CNF6);	// Alternate function output Open-drain
	/*PB7 - SDA*/
	GPIOB->CRL |= GPIO_CRL_MODE7;			// Output mode, max speed 50 MHz
	MODIFY_REG(GPIOB->CRL,GPIO_CRL_CNF7, GPIO_CRL_CNF7);	// Alternate function output Open-drain

	//____________ I2C Init Structure ____________//
	/* initialize the I2C_ClockSpeed member */
	I2C1_InitStruct.I2C_ClockSpeed = I2C_Freq;
	/* Initialize the I2C_Mode member */
	I2C1_InitStruct.I2C_Mode = I2C_Mode_I2C;
	/* Initialize the I2C_DutyCycle member */
	I2C1_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	/* Initialize the I2C_OwnAddress1 member */
	I2C1_InitStruct.I2C_OwnAddress1 = 0;
	/* Initialize the I2C_Ack member */
	I2C1_InitStruct.I2C_Ack = I2C_Ack_Enable;
	/* Initialize the I2C_AcknowledgedAddress member */
	I2C1_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	I2C_Init(I2C1, &I2C1_InitStruct);
	I2C_Cmd(I2C1, ENABLE);
}


//____________ Start / Stop ____________//
void I2C_StartCondition(I2C_TypeDef* I2Cx)
{
	SET_BIT(I2Cx->CR1, (1 << 8));	// Start
	while(!READ_BIT(I2Cx->SR1, (1 << 0))){};	// wait until Start condition is generated
	(void) I2Cx->SR1;
}

void I2C_StopCondition(I2C_TypeDef* I2Cx)
{
	SET_BIT(I2Cx->CR1, (1 << 9));
}
//____________ Send ____________//
void I2C_SendAddress(I2C_TypeDef* I2Cx, uint8_t Address)
{
	/* Write in the DR register the Addres to be sent */
	I2Cx->DR = Address;
	while(!READ_BIT(I2Cx->SR1, (1 << 1))){};	// wait until address, received by slave, is matched
	(void) I2Cx->SR1;
	(void) I2Cx->SR2;
}

void I2C_SendByte(I2C_TypeDef* I2Cx, uint8_t Byte)
{
	/* Write in the DR register the Byte to be sent */
	I2Cx->DR = Byte;
	while(!READ_BIT(I2Cx->SR1, (1 << 7))){};	// wait until Data register is empty
}

void I2C_SendByteToAddress(I2C_TypeDef* I2Cx, uint8_t Address, uint8_t Byte)
{
	I2C_StartCondition(I2C1);
	I2C_SendAddress(I2Cx, Address);
	I2C_SendByte(I2Cx, Byte);
	I2C_StopCondition(I2Cx);
}
//____________ Read ____________//
uint8_t I2C_ReadByte(I2C_TypeDef* I2Cx)
{
	uint8_t Byte;
	/* ACK: Acknowledge enabled */
	I2Cx->CR1 |= (1 << 10);
	while(!READ_BIT(I2Cx->SR1, (1 << 6))){};	// wait until Data register is full
	Byte = I2C_ReceiveData(I2Cx);

	return Byte;
}

uint8_t I2C_ReadLastByte(I2C_TypeDef* I2Cx)
{
	uint8_t Byte;
	/* ACK: Acknowledge disabled */
	I2Cx->CR1 &= ~(1 << 10);
	while(!READ_BIT(I2Cx->SR1, (1 << 6))){};	// wait until Data register is full
	Byte = I2C_ReceiveData(I2Cx);

	return Byte;
}