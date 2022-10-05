#include "uart.h"

uint32_t temp1, temp2;
extern RCC_ClocksTypeDef rcc_clocks;

#ifdef UART1_ENABLE
struct uart_str UART1STR;
#endif
#ifdef UART2_ENABLE
struct uart_str UART2STR;
#endif
#ifdef UART3_ENABLE
struct uart_str UART3STR;
#endif
#ifdef UART4_ENABLE
struct uart_str UART4STR;
#endif
#ifdef UART5_ENABLE
struct uart_str UART5STR;
#endif
#ifdef UART6_ENABLE
struct uart_str UART6STR;
#endif

// ========================================================================================================================== //
// =================================================== Инициализация UART =================================================== //
// ========================================================================================================================== //
void UART_Init(USART_TypeDef* USARTx, struct uart_str* UARTx, long Speed, char Data_bits, char Stop_bits, char parity, char mode)
{
	unsigned long tmp = 0;
	
	uint32_t UART_CLK_APB1 = rcc_clocks.PCLK1_Frequency;
	uint32_t UART_CLK_APB2 = rcc_clocks.PCLK2_Frequency;
	
	// ====== Инициализация структуры
	UARTx->Tx.Status = UART_BUSY_FREE;
	UARTx->Tx.Counter = 0;
	UARTx->Rx.Status = UART_STATUS_FREE;
	UARTx->Rx.Counter = 0;
	UARTx->TIM[0].CEN = UART_TIM_DISABLE;
	UARTx->TIM[0].CNT = 0;
	UARTx->TIM[0].Timeout = (unsigned short)((unsigned long)12 * 70000 / Speed); // 3.5 слова
	UARTx->TIM[1].CNT = 0;
	UARTx->TIM[1].TimeWord = (unsigned short)((unsigned long)12 * 24000 / Speed); // 20000 + 20%.
	UARTx->TIM[1].Timeout = 0;
	// ====== Настройка таймера TIM2
	if (!(TIM2->CR1 & TIM_CR1_CEN)) // если таймер уже настроен, то не инициализируем
	{
		// тактирование Basic Timers - TIM2
#ifdef UART_CPU_STM32H7xx
		RCC->APB1LENR |= RCC_APB1LENR_TIM7EN;
#else
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
#endif
		TIM2->PSC = 1 - 1; // Частота тактирования таймера // F_CK_PSC / (PSC + 1), 72 МГц / 1 = 72 МГц
		TIM2->ARR = (unsigned short)((long)UART_CLK_APB1 * UART_TIM_CLK_ML / 20000) - 1; // 72 МГц / 3600 = 20 кГц (50 мкс)
		TIM2->DIER |= TIM_DIER_UIE; // разрешаем прерывание от таймера
		TIM2->CR1 |= TIM_CR1_CEN | TIM_CR1_ARPE;
		//NVIC_SetPriority(TIM7_IRQn, 2);
		NVIC_EnableIRQ(TIM2_IRQn); // Разрешение TIM2_IRQn прерывания
	}
	// ====== Настройка UART
	// - включение тактирование блока uart, и расчет скорости
#ifdef UART1_ENABLE
	if (USARTx == USART1) {RCC->APB2ENR |= RCC_APB2ENR_USART1EN; tmp = (unsigned long)(UART_CLK_APB2 + Speed / 2) / Speed;}
#endif
#ifdef UART2_ENABLE
#ifdef UART_CPU_STM32H7xx
	if (USARTx == USART2) {RCC->APB1LENR |= RCC_APB1LENR_USART2EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#else
	if (USARTx == USART2) {RCC->APB1ENR |= RCC_APB1ENR_USART2EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#endif
#endif
#ifdef UART3_ENABLE
#ifdef UART_CPU_STM32H7xx
	if (USARTx == USART3) {RCC->APB1LENR |= RCC_APB1LENR_USART3EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#else
	if (USARTx == USART3) {RCC->APB1ENR |= RCC_APB1ENR_USART3EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#endif
#endif
#ifdef UART4_ENABLE
#ifdef UART_CPU_STM32H7xx
	if (USARTx == UART4) {RCC->APB1LENR |= RCC_APB1LENR_UART4EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#else
	if (USARTx == UART4) {RCC->APB1ENR |= RCC_APB1ENR_UART4EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#endif
#endif
#ifdef UART5_ENABLE
#ifdef UART_CPU_STM32H7xx
	if (USARTx == UART5) {RCC->APB1LENR |= RCC_APB1LENR_UART5EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#else
	if (USARTx == UART5) {RCC->APB1ENR |= RCC_APB1ENR_UART5EN; tmp = (unsigned long)(UART_CLK_APB1 + Speed / 2) / Speed;}
#endif
#endif
#ifdef UART6_ENABLE
	if (USARTx == USART6) {RCC->APB2ENR |= RCC_APB2ENR_USART6EN; tmp = (unsigned long)(UART_CLK_APB2 + Speed / 2) / Speed;}
#endif
	// - скорость
	USARTx->BRR = (unsigned short) tmp; // USART_BRR = (fck + baudrate /2 ) / baudrate
	// - Word length
	switch (Data_bits)
	{
	case UART_DATA_8BITS: USARTx->CR1 &= ~USART_CR1_M; break;
	case UART_DATA_9BITS: USARTx->CR1 |= USART_CR1_M; break;
	default: USARTx->CR1 &= ~USART_CR1_M; break; // по умолчанию 8 бит
	}
	// - STOP bits
	USARTx->CR2 &= ~USART_CR2_STOP;
	switch (Stop_bits)
	{
	case UART_STOP_05BIT: USARTx->CR2 |= USART_CR2_STOP_0; break;
	case UART_STOP_1BIT: USARTx->CR2 &= ~USART_CR2_STOP; break;
	case UART_STOP_15BIT: USARTx->CR2 |= USART_CR2_STOP; break;
	case UART_STOP_2BITS: USARTx->CR2 |= USART_CR2_STOP_1; break;
	default: USARTx->CR2 |= USART_CR2_STOP_1; break; // по умолчанию 1 стоп бит
	}
	// - Parity
	switch (parity)
	{
	case UART_NONE_PARITY: USARTx->CR1 &= ~USART_CR1_PCE; break;
	case UART_ODD_PARITY: USARTx->CR1 |= USART_CR1_PCE; USARTx->CR1 |= USART_CR1_PS; break;
	case UART_EVEN_PARITY: USARTx->CR1 |= USART_CR1_PCE; USARTx->CR1 &= ~USART_CR1_PS; break;
	default : USARTx->CR1 &= ~USART_CR1_PCE; break; // по умолчанию отключено
	}
	USARTx->CR1 |= USART_CR1_TE; // Transmitter enable, 1: Transmitter is enabled
	USARTx->CR1 |= USART_CR1_RE; // Receiver enable, 1: Receiver is enabled and begins searching for a start bit
	if (mode == UART_MODE_NORM)
	{
#ifdef UART_CPU_STM32F10x
		USARTx->CR1 |= USART_CR1_TCIE; // Transmission complete interrupt enable, 1: An USART interrupt is generated whenever TC=1 in the USART_SR register
		USARTx->CR1 |= USART_CR1_RXNEIE; // RXNE interrupt enable, 1: An USART interrupt is generated whenever ORE=1 or RXNE=1 in the USART_SR register
#endif
#ifdef UART_CPU_STM32F4xx
		USARTx->CR1 |= USART_CR1_TCIE; // Transmission complete interrupt enable, 1: An USART interrupt is generated whenever TC=1 in the USART_SR register
		USARTx->CR1 |= USART_CR1_RXNEIE; // RXNE interrupt enable, 1: An USART interrupt is generated whenever ORE=1 or RXNE=1 in the USART_SR register
#endif
#ifdef UART_CPU_STM32F3xx
		USARTx->CR1 |= USART_CR1_RXNEIE;
#endif
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
		USARTx->CR1 |= USART_CR1_TCIE;
		USARTx->CR1 |= USART_CR1_RXNEIE;
#endif
	}
	USARTx->CR1 |= USART_CR1_UE; // USART enable, 1: USART enabled
#ifdef UART1_ENABLE
	if ((USARTx == USART1) && (mode == UART_MODE_NORM)) NVIC_EnableIRQ(USART1_IRQn);
#endif
#ifdef UART2_ENABLE
	if ((USARTx == USART2) && (mode == UART_MODE_NORM)) NVIC_EnableIRQ(USART2_IRQn);
#endif
#ifdef UART3_ENABLE
	if ((USARTx == USART3) && (mode == UART_MODE_NORM)) NVIC_EnableIRQ(USART3_IRQn);
#endif
#ifdef UART4_ENABLE
	if ((USARTx == UART4) && (mode == UART_MODE_NORM)) NVIC_EnableIRQ(UART4_IRQn);
#endif
#ifdef UART5_ENABLE
	if ((USARTx == UART5) && (mode == UART_MODE_NORM)) NVIC_EnableIRQ(UART5_IRQn);
#endif
#ifdef UART6_ENABLE
	if ((USARTx == USART6) && (mode == UART_MODE_NORM)) NVIC_EnableIRQ(USART6_IRQn);
#endif
	
#ifdef UART1_RS485
	if (USARTx == USART1) UART1_DE_PORT->ODR &= ~UART1_DE_PIN; // На прием
#endif
#ifdef UART2_RS485
	if (USARTx == USART2) UART2_DE_PORT->ODR &= ~UART2_DE_PIN; // На прием
#endif
#ifdef UART3_RS485
	if (USARTx == USART3) UART3_DE_PORT->ODR &= ~UART3_DE_PIN; // На прием
#endif
#ifdef UART4_RS485
	if (USARTx == UART4) UART4_DE_PORT->ODR &= ~UART4_DE_PIN; // На прием
#endif
#ifdef UART5_RS485
	if (USARTx == UART5) UART5_DE_PORT->ODR &= ~UART5_DE_PIN; // На прием
#endif
#ifdef UART6_RS485
	if (USARTx == USART6) UART6_DE_PORT->ODR &= ~UART6_DE_PIN; // На прием
#endif
}

// ========================================================================================================================== //
// ================================================= Передача данных по UART ================================================ //
// ========================================================================================================================== //

// ========== Передача данных по прерыванию
// возвращать зачение что передача не возможна т.к. занят передачтки
unsigned int UART_Send_Interrupt(USART_TypeDef* USARTx, struct uart_str* UARTx, unsigned char *buffer, unsigned short count)
{
	if (UARTx->Tx.Status == UART_BUSY_FREE)
	{
		UARTx->TIM[1].Timeout = UARTx->TIM[1].TimeWord * count;
#ifdef UART1_RS485
		if (USARTx == USART1) UART1_DE_PORT->ODR |= UART1_DE_PIN; // Передача
#endif
#ifdef UART2_RS485
		if (USARTx == USART2) UART2_DE_PORT->ODR |= UART2_DE_PIN; // Передача
#endif
#ifdef UART3_RS485
		if (USARTx == USART3) UART3_DE_PORT->ODR |= UART3_DE_PIN; // Передача
#endif
#ifdef UART4_RS485
		if (USARTx == UART4) UART4_DE_PORT->ODR |= UART4_DE_PIN; // Передача
#endif
#ifdef UART5_RS485
		if (USARTx == UART5) UART5_DE_PORT->ODR |= UART5_DE_PIN; // Передача
#endif
#ifdef UART6_RS485
		if (USARTx == USART6) UART6_DE_PORT->ODR |= UART6_DE_PIN; // Передача
#endif
		UARTx->Tx.Counter = 0;
		UARTx->Tx.Status = UART_BUSY_SEND;
		UARTx->Tx.Length = count;
		if (count > MAX_UART_DATA) count = MAX_UART_DATA;
		memcpy(UARTx->Tx.Data, buffer, count);
		// Стартуем передачу данных
#ifdef UART_CPU_STM32F10x
		USARTx->DR = UARTx->Tx.Data[UARTx->Tx.Counter++];
#endif
#ifdef UART_CPU_STM32F4xx
		USARTx->DR = UARTx->Tx.Data[UARTx->Tx.Counter++];
#endif
#ifdef UART_CPU_STM32F3xx
		USARTx->CR1 |= USART_CR1_TXEIE;
#endif
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
		//USARTx->CR1 |= USART_CR1_TXEIE;
		USARTx->TDR = UARTx->Tx.Data[UARTx->Tx.Counter++];
#endif
		return 1; //
	} else return 0; // передатчик занят, передача не возможна
}

// ========================================================================================================================== //
// =================================================== Прием данных по UART ================================================= //
// ========================================================================================================================== //

// в функцию передовать указатель на массив, куда закидывать принятые байты
// а возвращать будет кол-во записываемых байт в массив
// если возвращает ноль, значит не пришло ничего.
unsigned int UART_Receive(struct uart_str* UARTx, unsigned char *buffer, unsigned short buffer_size)
{
	if (UARTx->Rx.Status == UART_STATUS_ACCEPT) // Пришли ли данные
	{
		unsigned short data_lgh = UARTx->Rx.Length;
		if (data_lgh > buffer_size) data_lgh = buffer_size;
#ifndef NEWLINE_BREAK
		memcpy(buffer, UARTx->Rx.Data, data_lgh);
#else
		memcpy(buffer, UARTx->Rx.Data, buffer_size);
#endif
		UARTx->Rx.Status = UART_STATUS_FREE;
		return data_lgh;
	}
	else return 0;
}

// ========================================================================================================================== //
// ==================================================== Прерывания по UART ================================================== //
// ========================================================================================================================== //

// ========== Прерывание по USART1
#ifdef UART1_ENABLE
void USART1_IRQHandler(void)
{
#ifdef UART_CPU_STM32F10x
	if (USART1->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART1STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART1STR.Rx.Counter < MAX_UART_DATA)
			{
				UART1STR.Rx.Status = UART_STATUS_RECV;
				UART1STR.Rx.Data[UART1STR.Rx.Counter++] = (unsigned char)USART1->DR;
				UART1STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART1STR.TIM[0].CEN = UART_TIM_ENABLE;
#ifdef NEWLINE_BREAK
				if((unsigned char)USART1->DR == '\n')
				{
					memset(UART1STR.Rx.Data + UART1STR.Rx.Counter, '\0', sizeof(UART1STR.Rx.Data) - UART1STR.Rx.Counter);
					UART1STR.TIM[0].CNT = UART1STR.TIM[0].Timeout; UART1STR.Rx.Status = UART_STATUS_ACCEPT;
				}
#endif					
			} else USART1->SR &= ~USART_SR_RXNE;
		} else USART1->SR &= ~USART_SR_RXNE;
	}
	if (USART1->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		USART1->SR &= ~USART_SR_TC;
		if (UART1STR.Tx.Counter < UART1STR.Tx.Length) USART1->DR = UART1STR.Tx.Data[UART1STR.Tx.Counter++];
		else
		{
			UART1STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART1_RS485
			UART1_DE_PORT->ODR &= ~UART1_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F4xx
	if (USART1->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART1STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART1STR.Rx.Counter < MAX_UART_DATA)
			{
				UART1STR.Rx.Status = UART_STATUS_RECV;
				UART1STR.Rx.Data[UART1STR.Rx.Counter++] = (unsigned char)USART1->DR;
				UART1STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART1STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART1->SR &= ~USART_SR_RXNE;
		} else USART1->SR &= ~USART_SR_RXNE;
	}
	if (USART1->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		USART1->SR &= ~USART_SR_TC;
		if (UART1STR.Tx.Counter < UART1STR.Tx.Length) USART1->DR = UART1STR.Tx.Data[UART1STR.Tx.Counter++];
		else
		{
			UART1STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART1_RS485
			UART1_DE_PORT->ODR &= ~UART1_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F3xx
	if (USART1->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		USART1->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (USART1->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
		if (UART1STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART1STR.Rx.Counter < MAX_UART_DATA)
			{
				UART1STR.Rx.Status = UART_STATUS_RECV;
				UART1STR.Rx.Data[UART1STR.Rx.Counter++] = (unsigned char)USART1->RDR;
				UART1STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART1STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART1->RDR; // сбросить флаг приема
		} else USART1->RDR; // сбросить флаг приема
	}
	if ((USART1->ISR & USART_ISR_TXE) && (USART1->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
		if (UART1STR.Tx.Counter <= UART1STR.Tx.Length) USART1->TDR = UART1STR.Tx.Data[UART1STR.Tx.Counter++];
		else
		{
			USART1->CR1 &= ~USART_CR1_TXEIE;
			UART1STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART1_RS485
			UART1_DE_PORT->ODR &= ~UART1_DE_PIN; // Прием
#endif
		}
	}
#endif
	
	/*#ifdef UART_CPU_STM32F7xx
	if (USART1->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
	USART1->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
}
	if (USART1->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
	if (UART1STR.Rx.Status != UART_STATUS_ACCEPT)
	{
	if (UART1STR.Rx.Counter < MAX_UART_DATA)
	{
	UART1STR.Rx.Status = UART_STATUS_RECV;
	UART1STR.Rx.Data[UART1STR.Rx.Counter++] = (unsigned char)USART1->RDR;
	UART1STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
	UART1STR.TIM[0].CEN = UART_TIM_ENABLE;
} else USART1->RDR; // сбросить флаг приема
} else USART1->RDR; // сбросить флаг приема
}
	if ((USART1->ISR & USART_ISR_TXE) && (USART1->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
	if (UART1STR.Tx.Counter < UART1STR.Tx.Length) USART1->TDR = UART1STR.Tx.Data[UART1STR.Tx.Counter++];
	else
	{
	USART1->CR1 &= ~USART_CR1_TXEIE;
	UART1STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART1_RS485
	UART1_DE_PORT->ODR &= ~UART1_DE_PIN; // Прием
#endif
}
}
#endif*/
	
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
	if (USART1->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		USART1->ICR = (USART_ICR_ORECF | USART_ICR_NECF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (USART1->ISR & USART_ISR_RXNE_RXFNE) // по окончанию приема очередного байта
	{
		if (UART1STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART1STR.Rx.Counter < MAX_UART_DATA)
			{
				UART1STR.Rx.Status = UART_STATUS_RECV;
				UART1STR.Rx.Data[UART1STR.Rx.Counter++] = (unsigned char)USART1->RDR;
				UART1STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART1STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART1->RDR; // сбросить флаг приема
		} else USART1->RDR; // сбросить флаг приема
	}
	if (USART1->ISR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		USART1->ICR = USART_ICR_TCCF;
		if (UART1STR.Tx.Counter < UART1STR.Tx.Length) USART1->TDR = UART1STR.Tx.Data[UART1STR.Tx.Counter++];
		else
		{
			UART1STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART1_RS485
			UART1_DE_PORT->ODR &= ~UART1_DE_PIN; // Прием
#endif
		}
	}
#endif
}
#endif

// ========== Прерывание по USART2
#ifdef UART2_ENABLE
void USART2_IRQHandler(void)
{
#ifdef UART_CPU_STM32F10x
	if (USART2->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART2STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART2STR.Rx.Counter < MAX_UART_DATA)
			{
				UART2STR.Rx.Status = UART_STATUS_RECV;
				UART2STR.Rx.Data[UART2STR.Rx.Counter++] = (unsigned char)USART2->DR;
				UART2STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART2STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART2->SR &= ~USART_SR_RXNE;
		} else USART2->SR &= ~USART_SR_RXNE;
	}
	if (USART2->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		USART2->SR &= ~USART_SR_TC;
		if (UART2STR.Tx.Counter < UART2STR.Tx.Length) USART2->DR = UART2STR.Tx.Data[UART2STR.Tx.Counter++];
		else
		{
			UART2STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART2_RS485
			UART2_DE_PORT->ODR &= ~UART2_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F4xx
	if (USART2->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART2STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART2STR.Rx.Counter < MAX_UART_DATA)
			{
				UART2STR.Rx.Status = UART_STATUS_RECV;
				UART2STR.Rx.Data[UART2STR.Rx.Counter++] = (unsigned char)USART2->DR;
				UART2STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART2STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART2->SR &= ~USART_SR_RXNE;
		} else USART2->SR &= ~USART_SR_RXNE;
	}
	if (USART2->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		USART2->SR &= ~USART_SR_TC;
		if (UART2STR.Tx.Counter < UART2STR.Tx.Length) USART2->DR = UART2STR.Tx.Data[UART2STR.Tx.Counter++];
		else
		{
			UART2STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART2_RS485
			UART2_DE_PORT->ODR &= ~UART2_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F3xx
	if (USART2->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		USART2->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (USART2->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
		if (UART2STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART2STR.Rx.Counter < MAX_UART_DATA)
			{
				UART2STR.Rx.Status = UART_STATUS_RECV;
				UART2STR.Rx.Data[UART2STR.Rx.Counter++] = (unsigned char)USART2->RDR;
				UART2STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART2STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART2->RDR;
		} else USART2->RDR;
	}
	if ((USART2->ISR & USART_ISR_TXE) && (USART2->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
		if (UART2STR.Tx.Counter <= UART2STR.Tx.Length) USART2->TDR = UART2STR.Tx.Data[UART2STR.Tx.Counter++];
		else
		{
			USART2->CR1 &= ~USART_CR1_TXEIE;
			UART2STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART2_RS485
			UART2_DE_PORT->ODR &= ~UART2_DE_PIN; // Прием
#endif
		}
	}
#endif
	
	/*#ifdef UART_CPU_STM32F7xx
	if (USART2->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
	USART2->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
}
	if (USART2->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
	if (UART2STR.Rx.Status != UART_STATUS_ACCEPT)
	{
	if (UART2STR.Rx.Counter < MAX_UART_DATA)
	{
	UART2STR.Rx.Status = UART_STATUS_RECV;
	UART2STR.Rx.Data[UART2STR.Rx.Counter++] = (unsigned char)USART2->RDR;
	UART2STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
	UART2STR.TIM[0].CEN = UART_TIM_ENABLE;
} else USART2->RDR;
} else USART2->RDR;
}
	if ((USART2->ISR & USART_ISR_TXE) && (USART2->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
	if (UART2STR.Tx.Counter < UART2STR.Tx.Length) USART2->TDR = UART2STR.Tx.Data[UART2STR.Tx.Counter++];
	else
	{
	USART2->CR1 &= ~USART_CR1_TXEIE;
	UART2STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART2_RS485
	UART1_DE_PORT->ODR &= ~UART1_DE_PIN; // Прием
#endif
}
}
#endif*/
	
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
	if (USART2->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		USART2->ICR = (USART_ICR_ORECF | USART_ICR_NECF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (USART2->ISR & USART_ISR_RXNE_RXFNE) // по окончанию приема очередного байта
	{
		if (UART2STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART2STR.Rx.Counter < MAX_UART_DATA)
			{
				UART2STR.Rx.Status = UART_STATUS_RECV;
				UART2STR.Rx.Data[UART2STR.Rx.Counter++] = (unsigned char)USART2->RDR;
				UART2STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART2STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART2->RDR;
		} else USART2->RDR;
	}
	if (USART2->ISR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		USART2->ICR = USART_ICR_TCCF;
		if (UART2STR.Tx.Counter < UART2STR.Tx.Length) USART2->TDR = UART2STR.Tx.Data[UART2STR.Tx.Counter++];
		else
		{
			UART2STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART2_RS485
			UART2_DE_PORT->ODR &= ~UART2_DE_PIN; // Прием
#endif
		}
	}
#endif
}
#endif

// ========== Прерывание по USART3
#ifdef UART3_ENABLE
void USART3_IRQHandler(void)
{
#ifdef UART_CPU_STM32F10x
	if (USART3->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART3STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART3STR.Rx.Counter < MAX_UART_DATA)
			{
				UART3STR.Rx.Status = UART_STATUS_RECV;
				UART3STR.Rx.Data[UART3STR.Rx.Counter++] = (unsigned char)USART3->DR;
				UART3STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART3STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART3->SR &= ~USART_SR_RXNE;
		} else USART3->SR &= ~USART_SR_RXNE;
	}
	if (USART3->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		USART3->SR &= ~USART_SR_TC;
		if (UART3STR.Tx.Counter < UART3STR.Tx.Length) USART3->DR = UART3STR.Tx.Data[UART3STR.Tx.Counter++];
		else
		{
			UART3STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART3_RS485
			UART3_DE_PORT->ODR &= ~UART3_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F4xx
	if (USART3->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART3STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART3STR.Rx.Counter < MAX_UART_DATA)
			{
				UART3STR.Rx.Status = UART_STATUS_RECV;
				UART3STR.Rx.Data[UART3STR.Rx.Counter++] = (unsigned char)USART3->DR;
				UART3STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART3STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART3->SR &= ~USART_SR_RXNE;
		} else USART3->SR &= ~USART_SR_RXNE;
	}
	if (USART3->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		USART3->SR &= ~USART_SR_TC;
		if (UART3STR.Tx.Counter < UART3STR.Tx.Length) USART3->DR = UART3STR.Tx.Data[UART3STR.Tx.Counter++];
		else
		{
			UART3STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART3_RS485
			UART3_DE_PORT->ODR &= ~UART3_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F3xx
	if (USART3->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		USART3->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (USART3->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
		if (UART3STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART3STR.Rx.Counter < MAX_UART_DATA)
			{
				UART3STR.Rx.Status = UART_STATUS_RECV;
				UART3STR.Rx.Data[UART3STR.Rx.Counter++] = (unsigned char)USART3->RDR;
				UART3STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART3STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART3->RDR;
		} else USART3->RDR;
	}
	if ((USART3->ISR & USART_ISR_TXE) && (USART3->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
		if (UART3STR.Tx.Counter <= UART3STR.Tx.Length) USART3->TDR = UART3STR.Tx.Data[UART3STR.Tx.Counter++];
		else
		{
			USART3->CR1 &= ~USART_CR1_TXEIE;
			UART3STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART3_RS485
			UART3_DE_PORT->ODR &= ~UART3_DE_PIN; // Прием
#endif
		}
	}
#endif
	
	/*#ifdef UART_CPU_STM32F7xx
	if (USART3->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
	USART3->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
}
	if (USART3->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
	if (UART3STR.Rx.Status != UART_STATUS_ACCEPT)
	{
	if (UART3STR.Rx.Counter < MAX_UART_DATA)
	{
	UART3STR.Rx.Status = UART_STATUS_RECV;
	UART3STR.Rx.Data[UART3STR.Rx.Counter++] = (unsigned char)USART3->RDR;
	UART3STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
	UART3STR.TIM[0].CEN = UART_TIM_ENABLE;
} else USART3->RDR;
} else USART3->RDR;
}
	if ((USART3->ISR & USART_ISR_TXE) && (USART3->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
	if (UART3STR.Tx.Counter < UART3STR.Tx.Length) USART3->TDR = UART3STR.Tx.Data[UART3STR.Tx.Counter++];
	else
	{
	USART3->CR1 &= ~USART_CR1_TXEIE;
	UART3STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART3_RS485
	UART3_DE_PORT->ODR &= ~UART3_DE_PIN; // Прием
#endif
}
}
#endif*/
	
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
	if (USART3->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		USART3->ICR = (USART_ICR_ORECF | USART_ICR_NECF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (USART3->ISR & USART_ISR_RXNE_RXFNE) // по окончанию приема очередного байта
	{
		if (UART3STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART3STR.Rx.Counter < MAX_UART_DATA)
			{
				UART3STR.Rx.Status = UART_STATUS_RECV;
				UART3STR.Rx.Data[UART3STR.Rx.Counter++] = (unsigned char)USART3->RDR;
				UART3STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART3STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART3->RDR;
		} else USART3->RDR;
	}
	if (USART3->ISR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		USART3->ICR = USART_ICR_TCCF;
		if (UART3STR.Tx.Counter < UART3STR.Tx.Length) USART3->TDR = UART3STR.Tx.Data[UART3STR.Tx.Counter++];
		else
		{
			UART3STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART3_RS485
			UART3_DE_PORT->ODR &= ~UART3_DE_PIN; // Прием
#endif
		}
	}
#endif
}
#endif

// ========== Прерывание по UART4
#ifdef UART4_ENABLE
void UART4_IRQHandler(void)
{
#ifdef UART_CPU_STM32F10x
	if (UART4->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART4STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART4STR.Rx.Counter < MAX_UART_DATA)
			{
				UART4STR.Rx.Status = UART_STATUS_RECV;
				UART4STR.Rx.Data[UART4STR.Rx.Counter++] = (unsigned char)UART4->DR;
				UART4STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART4STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART4->SR &= ~USART_SR_RXNE;
		} else UART4->SR &= ~USART_SR_RXNE;
	}
	if (UART4->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		UART4->SR &= ~USART_SR_TC;
		if (UART4STR.Tx.Counter < UART4STR.Tx.Length) UART4->DR = UART4STR.Tx.Data[UART4STR.Tx.Counter++];
		else
		{
			UART4STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART4_RS485
			UART4_DE_PORT->ODR &= ~UART4_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F4xx
	if (UART4->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART4STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART4STR.Rx.Counter < MAX_UART_DATA)
			{
				UART4STR.Rx.Status = UART_STATUS_RECV;
				UART4STR.Rx.Data[UART4STR.Rx.Counter++] = (unsigned char)UART4->DR;
				UART4STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART4STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART4->SR &= ~USART_SR_RXNE;
		} else UART4->SR &= ~USART_SR_RXNE;
	}
	if (UART4->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		UART4->SR &= ~USART_SR_TC;
		if (UART4STR.Tx.Counter < UART4STR.Tx.Length) UART4->DR = UART4STR.Tx.Data[UART4STR.Tx.Counter++];
		else
		{
			UART4STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART4_RS485
			UART4_DE_PORT->ODR &= ~UART4_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F3xx
	if (UART4->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		UART4->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (UART4->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
		if (UART4STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART4STR.Rx.Counter < MAX_UART_DATA)
			{
				UART4STR.Rx.Status = UART_STATUS_RECV;
				UART4STR.Rx.Data[UART4STR.Rx.Counter++] = (unsigned char)UART4->RDR;
				UART4STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART4STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART4->RDR;
		} else UART4->RDR;
	}
	if ((UART4->ISR & USART_ISR_TXE) && (UART4->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
		if (UART4STR.Tx.Counter <= UART4STR.Tx.Length) UART4->TDR = UART4STR.Tx.Data[UART4STR.Tx.Counter++];
		else
		{
			UART4->CR1 &= ~USART_CR1_TXEIE;
			UART4STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART4_RS485
			UART4_DE_PORT->ODR &= ~UART4_DE_PIN; // Прием
#endif
		}
	}
#endif
	
	/*#ifdef UART_CPU_STM32F7xx
	if (UART4->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
	UART4->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
}
	if (UART4->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
	if (UART4STR.Rx.Status != UART_STATUS_ACCEPT)
	{
	if (UART4STR.Rx.Counter < MAX_UART_DATA)
	{
	UART4STR.Rx.Status = UART_STATUS_RECV;
	UART4STR.Rx.Data[UART4STR.Rx.Counter++] = (unsigned char)UART4->RDR;
	UART4STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
	UART4STR.TIM[0].CEN = UART_TIM_ENABLE;
} else UART4->RDR;
} else UART4->RDR;
}
	if ((UART4->ISR & USART_ISR_TXE) && (UART4->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
	if (UART4STR.Tx.Counter < UART4STR.Tx.Length) UART4->TDR = UART4STR.Tx.Data[UART4STR.Tx.Counter++];
	else
	{
	UART4->CR1 &= ~USART_CR1_TXEIE;
	UART4STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART4_RS485
	UART4_DE_PORT->ODR &= ~UART4_DE_PIN; // Прием
#endif
}
}
#endif*/
	
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
	if (UART4->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		UART4->ICR = (USART_ICR_ORECF | USART_ICR_NECF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (UART4->ISR & USART_ISR_RXNE_RXFNE) // по окончанию приема очередного байта
	{
		if (UART4STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART4STR.Rx.Counter < MAX_UART_DATA)
			{
				UART4STR.Rx.Status = UART_STATUS_RECV;
				UART4STR.Rx.Data[UART4STR.Rx.Counter++] = (unsigned char)UART4->RDR;
				UART4STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART4STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART4->RDR;
		} else UART4->RDR;
	}
	if (UART4->ISR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		UART4->ICR = USART_ICR_TCCF;
		if (UART4STR.Tx.Counter < UART4STR.Tx.Length) UART4->TDR = UART4STR.Tx.Data[UART4STR.Tx.Counter++];
		else
		{
			UART4STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART4_RS485
			UART4_DE_PORT->ODR &= ~UART4_DE_PIN; // Прием
#endif
		}
	}
#endif
}
#endif

// ========== Прерывание по UART5
#ifdef UART5_ENABLE
void UART5_IRQHandler(void)
{
#ifdef UART_CPU_STM32F10x
	if (UART5->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART5STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART5STR.Rx.Counter < MAX_UART_DATA)
			{
				UART5STR.Rx.Status = UART_STATUS_RECV;
				UART5STR.Rx.Data[UART5STR.Rx.Counter++] = (unsigned char)UART5->DR;
				UART5STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART5STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART5->SR &= ~USART_SR_RXNE;
		} else UART5->SR &= ~USART_SR_RXNE;
	}
	if (UART5->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		UART5->SR &= ~USART_SR_TC;
		if (UART5STR.Tx.Counter < UART5STR.Tx.Length) UART5->DR = UART5STR.Tx.Data[UART5STR.Tx.Counter++];
		else
		{
			UART5STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART5_RS485
			UART5_DE_PORT->ODR &= ~UART5_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F4xx
	if (UART5->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART5STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART5STR.Rx.Counter < MAX_UART_DATA)
			{
				UART5STR.Rx.Status = UART_STATUS_RECV;
				UART5STR.Rx.Data[UART5STR.Rx.Counter++] = (unsigned char)UART5->DR;
				UART5STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART5STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART5->SR &= ~USART_SR_RXNE;
		} else UART5->SR &= ~USART_SR_RXNE;
	}
	if (UART5->SR & USART_SR_TC) // прерывание по окончанию передачи
	{
		UART5->SR &= ~USART_SR_TC;
		if (UART5STR.Tx.Counter < UART5STR.Tx.Length) UART5->DR = UART5STR.Tx.Data[UART5STR.Tx.Counter++];
		else
		{
			UART5STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART5_RS485
			UART5_DE_PORT->ODR &= ~UART5_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F3xx
	if (UART5->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		UART5->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (UART5->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
		if (UART5STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART5STR.Rx.Counter < MAX_UART_DATA)
			{
				UART5STR.Rx.Status = UART_STATUS_RECV;
				UART5STR.Rx.Data[UART5STR.Rx.Counter++] = (unsigned char)UART5->RDR;
				UART5STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART5STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART5->RDR;
		} else UART5->RDR;
	}
	if ((UART5->ISR & USART_ISR_TXE) && (UART5->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
		if (UART5STR.Tx.Counter <= UART5STR.Tx.Length) UART5->TDR = UART5STR.Tx.Data[UART5STR.Tx.Counter++];
		else
		{
			UART5->CR1 &= ~USART_CR1_TXEIE;
			UART5STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART5_RS485
			UART5_DE_PORT->ODR &= ~UART5_DE_PIN; // Прием
#endif
		}
	}
#endif
	
	/*#ifdef UART_CPU_STM32F7xx
	if (UART5->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
	UART5->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
}
	if (UART5->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
	if (UART5STR.Rx.Status != UART_STATUS_ACCEPT)
	{
	if (UART5STR.Rx.Counter < MAX_UART_DATA)
	{
	UART5STR.Rx.Status = UART_STATUS_RECV;
	UART5STR.Rx.Data[UART5STR.Rx.Counter++] = (unsigned char)UART5->RDR;
	UART5STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
	UART5STR.TIM[0].CEN = UART_TIM_ENABLE;
} else UART5->RDR;
} else UART5->RDR;
}
	if ((UART5->ISR & USART_ISR_TXE) && (UART5->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
	if (UART5STR.Tx.Counter < UART5STR.Tx.Length) UART5->TDR = UART5STR.Tx.Data[UART5STR.Tx.Counter++];
	else
	{
	UART5->CR1 &= ~USART_CR1_TXEIE;
	UART5STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART5_RS485
	UART5_DE_PORT->ODR &= ~UART5_DE_PIN; // Прием
#endif
}
}
#endif*/
	
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
	if (UART5->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		UART5->ICR = (USART_ICR_ORECF | USART_ICR_NECF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (UART5->ISR & USART_ISR_RXNE_RXFNE) // по окончанию приема очередного байта
	{
		if (UART5STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART5STR.Rx.Counter < MAX_UART_DATA)
			{
				UART5STR.Rx.Status = UART_STATUS_RECV;
				UART5STR.Rx.Data[UART5STR.Rx.Counter++] = (unsigned char)UART5->RDR;
				UART5STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART5STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else UART5->RDR;
		} else UART5->RDR;
	}
	if (UART5->ISR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		UART5->ICR = USART_ICR_TCCF;
		if (UART5STR.Tx.Counter < UART5STR.Tx.Length) UART5->TDR = UART5STR.Tx.Data[UART5STR.Tx.Counter++];
		else
		{
			UART5STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART5_RS485
			UART5_DE_PORT->ODR &= ~UART5_DE_PIN; // Прием
#endif
		}
	}
#endif
}
#endif

// ========== Прерывание по USART6
#ifdef UART6_ENABLE
void USART6_IRQHandler(void)
{
#ifdef UART_CPU_STM32F10x
	if (USART6->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART6STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART6STR.Rx.Counter < MAX_UART_DATA)
			{
				UART6STR.Rx.Status = UART_STATUS_RECV;
				UART6STR.Rx.Data[UART6STR.Rx.Counter++] = (unsigned char)USART6->DR;
				UART6STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART6STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART6->SR &= ~USART_SR_RXNE;
		} else USART6->SR &= ~USART_SR_RXNE;
	}
	if (USART6->SR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		USART6->SR &= ~USART_SR_TC;
		if (UART6STR.Tx.Counter < UART6STR.Tx.Length) USART6->DR = UART6STR.Tx.Data[UART6STR.Tx.Counter++];
		else
		{
			UART6STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART6_RS485
			UART6_DE_PORT->ODR &= ~UART6_DE_PIN; // Прием
#endif
		}
	}
#endif
	
#ifdef UART_CPU_STM32F4xx
	if (USART6->SR & USART_SR_RXNE) // по окончанию приема очередного байта
	{
		if (UART6STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART6STR.Rx.Counter < MAX_UART_DATA)
			{
				UART6STR.Rx.Status = UART_STATUS_RECV;
				UART6STR.Rx.Data[UART6STR.Rx.Counter++] = (unsigned char)USART6->DR;
				UART6STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART6STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART6->SR &= ~USART_SR_RXNE;
		} else USART6->SR &= ~USART_SR_RXNE;
	}
	if (USART6->SR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		USART6->SR &= ~USART_SR_TC;
		if (UART6STR.Tx.Counter < UART6STR.Tx.Length) USART6->DR = UART6STR.Tx.Data[UART6STR.Tx.Counter++];
		else
		{
			UART6STR.Tx.Busy = UART_BUSY_FREE;
#ifdef UART6_RS485
			UART6_DE_PORT->ODR &= ~UART6_DE_PIN; // Прием
#endif
		}
	}
#endif
	
	/*#ifdef UART_CPU_STM32F7xx
	if (USART6->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
	USART6->ICR = (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
}
	if (USART6->ISR & USART_ISR_RXNE) // по окончанию приема очередного байта
	{
	if (UART6STR.Rx.Status != UART_STATUS_ACCEPT)
	{
	if (UART6STR.Rx.Counter < MAX_UART_DATA)
	{
	UART6STR.Rx.Status = UART_STATUS_RECV;
	UART6STR.Rx.Data[UART6STR.Rx.Counter++] = (unsigned char)USART6->RDR;
	UART6STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
	UART6STR.TIM[0].CEN = UART_TIM_ENABLE;
} else USART6->RDR;
} else USART6->RDR;
}
	if ((USART6->ISR & USART_ISR_TXE) && (USART6->CR1 & USART_CR1_TXEIE)) // прерывание по окончанию передачи
	{
	if (UART6STR.Tx.Counter < UART6STR.Tx.Length) USART6->TDR = UART6STR.Tx.Data[UART6STR.Tx.Counter++];
	else
	{
	USART6->CR1 &= ~USART_CR1_TXEIE;
	UART6STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART6_RS485
	UART6_DE_PORT->ODR &= ~UART6_DE_PIN; // Прием
#endif
}
}
#endif*/
	
#if defined(UART_CPU_STM32F7xx) || defined(UART_CPU_STM32H7xx)
	if (USART6->ISR & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE))
	{
		USART6->ICR = (USART_ICR_ORECF | USART_ICR_NECF | USART_ICR_FECF | USART_ICR_PECF);
	}
	if (USART6->ISR & USART_ISR_RXNE_RXFNE) // по окончанию приема очередного байта
	{
		if (UART6STR.Rx.Status != UART_STATUS_ACCEPT)
		{
			if (UART6STR.Rx.Counter < MAX_UART_DATA)
			{
				UART6STR.Rx.Status = UART_STATUS_RECV;
				UART6STR.Rx.Data[UART6STR.Rx.Counter++] = (unsigned char)USART6->RDR;
				UART6STR.TIM[0].CNT = 0; // Включаем таймер для отсчета таймаута
				UART6STR.TIM[0].CEN = UART_TIM_ENABLE;
			} else USART6->RDR;
		} else USART6->RDR;
	}
	if (USART6->ISR & USART_ISR_TC) // прерывание по окончанию передачи
	{
		USART6->ICR = USART_ICR_TCCF;
		if (UART6STR.Tx.Counter < UART6STR.Tx.Length) USART6->TDR = UART6STR.Tx.Data[UART6STR.Tx.Counter++];
		else
		{
			UART6STR.Tx.Status = UART_BUSY_FREE;
#ifdef UART6_RS485
			UART6_DE_PORT->ODR &= ~UART6_DE_PIN; // Прием
#endif
		}
	}
#endif
}
#endif

// ========================================================================================================================== //
// ===================================================== Таймауты по UART =================================================== //
// ========================================================================================================================== //

// ========== Прерывание по TIM2
void TIM2_IRQHandler(void)
{
	TIM2->SR &= ~TIM_SR_UIF; // сбросить флаг
	
#ifdef UART1_ENABLE
	if (UART1STR.TIM[0].CEN == UART_TIM_ENABLE) // UART1
	{
		if (UART1STR.TIM[0].CNT < UART1STR.TIM[0].Timeout) UART1STR.TIM[0].CNT++;
		else
		{
			UART1STR.TIM[0].CEN = UART_TIM_DISABLE;
			UART1STR.Rx.Status = UART_STATUS_ACCEPT;
			UART1STR.Rx.Length = UART1STR.Rx.Counter;
			UART1STR.Rx.Counter = 0;
		}
	}
#ifdef UART1_RS485
	if (UART1_DE_PORT->IDR & UART1_DE_PIN)
	{
		if (UART1STR.TIM[1].CNT < UART1STR.TIM[1].Timeout) UART1STR.TIM[1].CNT++;
		else
		{
			UART1STR.TIM[1].CNT = 0;
			UART1STR.Tx.Status = UART_BUSY_FREE;
			UART1_DE_PORT->ODR &= ~UART1_DE_PIN;
		}
	} else UART1STR.TIM[1].CNT = 0;
#endif
#endif
	
#ifdef UART2_ENABLE
	if (UART2STR.TIM[0].CEN == UART_TIM_ENABLE) // UART2
	{
		if (UART2STR.TIM[0].CNT < UART2STR.TIM[0].Timeout) UART2STR.TIM[0].CNT++;
		else
		{
			UART2STR.TIM[0].CEN = UART_TIM_DISABLE;
			UART2STR.Rx.Status = UART_STATUS_ACCEPT;
			UART2STR.Rx.Length = UART2STR.Rx.Counter;
			UART2STR.Rx.Counter = 0;
		}
	}
#ifdef UART2_RS485
	if (UART2_DE_PORT->IDR & UART2_DE_PIN)
	{
		if (UART2STR.TIM[1].CNT < UART2STR.TIM[1].Timeout) UART2STR.TIM[1].CNT++;
		else
		{
			UART2STR.TIM[1].CNT = 0;
			UART2STR.Tx.Status = UART_BUSY_FREE;
			UART2_DE_PORT->ODR &= ~UART2_DE_PIN;
		}
	} else UART2STR.TIM[1].CNT = 0;
#endif
#endif
#ifdef UART3_ENABLE
	if (UART3STR.TIM[0].CEN == UART_TIM_ENABLE) // UART3
	{
		if (UART3STR.TIM[0].CNT < UART3STR.TIM[0].Timeout) UART3STR.TIM[0].CNT++;
		else
		{
			UART3STR.TIM[0].CEN = UART_TIM_DISABLE;
			UART3STR.Rx.Status = UART_STATUS_ACCEPT;
			UART3STR.Rx.Length = UART3STR.Rx.Counter;
			UART3STR.Rx.Counter = 0;
		}
	}
#ifdef UART3_RS485
	if (UART3_DE_PORT->IDR & UART3_DE_PIN)
	{
		if (UART3STR.TIM[1].CNT < UART3STR.TIM[1].Timeout) UART3STR.TIM[1].CNT++;
		else
		{
			UART3STR.TIM[1].CNT = 0;
			UART3STR.Tx.Status = UART_BUSY_FREE;
			UART3_DE_PORT->ODR &= ~UART3_DE_PIN;
		}
	} else UART3STR.TIM[1].CNT = 0;
#endif
#endif
#ifdef UART4_ENABLE
	if (UART4STR.TIM[0].CEN == UART_TIM_ENABLE) // UART4
	{
		if (UART4STR.TIM[0].CNT < UART4STR.TIM[0].Timeout) UART4STR.TIM[0].CNT++;
		else
		{
			UART4STR.TIM[0].CEN = UART_TIM_DISABLE;
			UART4STR.Rx.Status = UART_STATUS_ACCEPT;
			UART4STR.Rx.Length = UART4STR.Rx.Counter;
			UART4STR.Rx.Counter = 0;
		}
	}
#ifdef UART4_RS485
	if (UART4_DE_PORT->IDR & UART4_DE_PIN)
	{
		if (UART4STR.TIM[1].CNT < UART4STR.TIM[1].Timeout) UART4STR.TIM[1].CNT++;
		else
		{
			UART4STR.TIM[1].CNT = 0;
			UART4STR.Tx.Status = UART_BUSY_FREE;
			UART4_DE_PORT->ODR &= ~UART4_DE_PIN;
		}
	} else UART4STR.TIM[1].CNT = 0;
#endif
#endif
#ifdef UART5_ENABLE
	if (UART5STR.TIM[0].CEN == UART_TIM_ENABLE) // UART5
	{
		if (UART5STR.TIM[0].CNT < UART5STR.TIM[0].Timeout) UART5STR.TIM[0].CNT++;
		else
		{
			UART5STR.TIM[0].CEN = UART_TIM_DISABLE;
			UART5STR.Rx.Status = UART_STATUS_ACCEPT;
			UART5STR.Rx.Length = UART5STR.Rx.Counter;
			UART5STR.Rx.Counter = 0;
		}
	}
#ifdef UART5_RS485
	if (UART5_DE_PORT->IDR & UART5_DE_PIN)
	{
		if (UART5STR.TIM[1].CNT < UART5STR.TIM[1].Timeout) UART5STR.TIM[1].CNT++;
		else
		{
			UART5STR.TIM[1].CNT = 0;
			UART5STR.Tx.Status = UART_BUSY_FREE;
			UART5_DE_PORT->ODR &= ~UART5_DE_PIN;
		}
	} else UART5STR.TIM[1].CNT = 0;
#endif
#endif
	
#ifdef UART6_ENABLE
	if (UART6STR.TIM[0].CEN == UART_TIM_ENABLE) // UART6
	{
		if (UART6STR.TIM[0].CNT < UART6STR.TIM[0].Timeout) UART6STR.TIM[0].CNT++;
		else
		{
			UART6STR.TIM[0].CEN = UART_TIM_DISABLE;
			UART6STR.Rx.Status = UART_STATUS_ACCEPT;
			UART6STR.Rx.Length = UART6STR.Rx.Counter;
			UART6STR.Rx.Counter = 0;
		}
	}
#ifdef UART6_RS485
	if (UART6_DE_PORT->IDR & UART6_DE_PIN)
	{
		if (UART6STR.TIM[1].CNT < UART6STR.TIM[1].Timeout) UART6STR.TIM[1].CNT++;
		else
		{
			UART6STR.TIM[1].CNT = 0;
			UART6STR.Tx.Status = UART_BUSY_FREE;
			UART6_DE_PORT->ODR &= ~UART6_DE_PIN; // На прием
		}
	} else UART6STR.TIM[1].CNT = 0;
#endif
#endif
}

// ========================================================================================================================== //
// ======================================================== print UART ====================================================== //
// ========================================================================================================================== //

// ========== 
/*void prints(USART_TypeDef* USARTx, char *str)
{
#ifdef UART_CPU_STM32F10x
unsigned short Len_txt, i = 0;
if (!(USARTx->CR1 & USART_CR1_UE)) return;
Len_txt = strlen(str);
USARTx->DR = str[i++]; USARTx->SR = 0x0000;
for (; i < Len_txt; i++)
{
while(!(USARTx->SR & USART_SR_TC));
USARTx->DR = str[i]; USARTx->SR = 0x0000;
		}
while(!(USARTx->SR & USART_SR_TC));
#endif

#ifdef UART_CPU_STM32F4xx
unsigned short Len_txt, i = 0;
if (!(USARTx->CR1 & USART_CR1_UE)) return;
Len_txt = strlen(str);
USARTx->DR = str[i++]; USARTx->SR = 0x0000;
for (; i < Len_txt; i++)
{
while(!(USARTx->SR & USART_SR_TC));
USARTx->DR = str[i]; USARTx->SR = 0x0000;
		}
while(!(USARTx->SR & USART_SR_TC));
#endif

#ifdef UART_CPU_STM32F3xx
unsigned short Len_txt, i = 0;
if (!(USARTx->CR1 & USART_CR1_UE)) return;
Len_txt = strlen(str);
USARTx->TDR = str[i++]; USARTx->ICR = 0xFFFF;
for (; i < Len_txt; i++)
{
while(!(USARTx->ISR & USART_ISR_TC));
USARTx->TDR = str[i]; USARTx->ICR = 0xFFFF;
		}
while(!(USARTx->ISR & USART_ISR_TC));
#endif
}*/

// использовать переменную cnt, в форе дикрементить
/*void printsc(USART_TypeDef* USARTx, char *str, unsigned short cnt)
{
#ifdef UART_CPU_STM32F10x
unsigned short i = 0;
if (!(USARTx->CR1 & USART_CR1_UE)) return;
USARTx->DR = str[i++]; USARTx->SR = 0x0000;
for (; i < cnt; i++)
{
while(!(USARTx->SR & USART_SR_TC));
USARTx->DR = str[i]; USARTx->SR = 0x0000;
		}
while(!(USARTx->SR & USART_SR_TC));
#endif

#ifdef UART_CPU_STM32F4xx
unsigned short i = 0;
if (!(USARTx->CR1 & USART_CR1_UE)) return;
USARTx->DR = str[i++]; USARTx->SR = 0x0000;
for (; i < cnt; i++)
{
while(!(USARTx->SR & USART_SR_TC));
USARTx->DR = str[i]; USARTx->SR = 0x0000;
		}
while(!(USARTx->SR & USART_SR_TC));
#endif
}*/

// принимает указатель на число unsigned char (0 - 255)
// возвращает 1 - если пришли данные (число + \r)
// возвращает 0 - либо ничего нет, либо мусор
/*char scansi(USART_TypeDef* USARTx, unsigned char *num)
{
#ifdef UART_CPU_STM32F10x
unsigned char tmp_dr = 0;
static unsigned char tmp[3] = {0, 0, 0}, cnt = 0;

if (!(USARTx->CR1 & USART_CR1_UE)) return 0;
if (USARTx->SR & USART_SR_RXNE)
{
tmp_dr = USARTx->DR;
if (tmp_dr == '\r')
{
if (cnt == 0) return 0;
switch (cnt)
{
					case 1: *num = (tmp[0] - 48); break;
					case 2: *num = (tmp[0] - 48) * 10 + (tmp[1] - 48); break;
					case 3: *num = (tmp[0] - 48) * 100 + (tmp[1] - 48) * 10 + (tmp[2] - 48); break;
				}
cnt = 0;
return 1;
			}
if ((tmp_dr >= 48) && (tmp_dr <= 57)) // записываем в массив только цифр
{
if (cnt < sizeof(tmp)) tmp[cnt++] = tmp_dr;
			}
		}
#endif

#ifdef UART_CPU_STM32F4xx
unsigned char tmp_dr = 0;
static unsigned char tmp[3] = {0, 0, 0}, cnt = 0;

if (!(USARTx->CR1 & USART_CR1_UE)) return 0;
if (USARTx->SR & USART_SR_RXNE)
{
tmp_dr = USARTx->DR;
if (tmp_dr == '\r')
{
if (cnt == 0) return 0;
switch (cnt)
{
					case 1: *num = (tmp[0] - 48); break;
					case 2: *num = (tmp[0] - 48) * 10 + (tmp[1] - 48); break;
					case 3: *num = (tmp[0] - 48) * 100 + (tmp[1] - 48) * 10 + (tmp[2] - 48); break;
				}
cnt = 0;
return 1;
			}
if ((tmp_dr >= 48) && (tmp_dr <= 57)) // записываем в массив только цифр
{
if (cnt < sizeof(tmp)) tmp[cnt++] = tmp_dr;
			}
		}
#endif
return 0;
}*/

/*char scansf(USART_TypeDef* USARTx, char *str) // читает по одному символу
{
if (!(USARTx->CR1 & USART_CR1_UE)) return;
if (USARTx->SR & USART_SR_RXNE)
{
*str = USARTx->DR;
return 1;
	}
return 0;
}*/
