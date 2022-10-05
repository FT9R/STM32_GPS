#ifndef uart_header
#define uart_header

//#define NEWLINE_BREAK		// receive only until '\n' is reached; Further content of UART1STR.Rx.Data[] after '\n' will be cleared each time

// ============================================================================================================================= //
// ========================================================== �������� ========================================================= //
// ============================================================================================================================= //

// ������ ��� uart, �.�. ��� ������ spi
// ������ ���� Tx, Rx � ��� RTS � CTS

// --------------
// �������������� ����������: ------
// ��� ������� ��������� ������������ ������ TIM2 (20 ��� (50 ���))

// ============================================================================================================================= //
// ====================================================== Revision history ===================================================== //
// ============================================================================================================================= //

// v0.2

// v0.3
// ������� ������ TIM7 ��� ������� ���������.

// v0.4
// �������� ���������� ��� 6 ������

// v0.5
// ������� ���� �����: �������� ���-�� ���� �����, �������� ��������
// ������� ����� ������ ������
// UART_MODE_PRINT ��� ������� printt, printl (�� ���������������� ����������)

// v0.6

// v0.7

// v0.8

// v0.9
// ���������� ������� UART_Recv (����� ������ �� uart)

// v0.10

// v0.11

// v0.12

// v0.13
// ���� ���-�� ����������� � ������������ ���� ����� ���� 65535

// v0.14
// ������� RS485

// v0.15
// ������� scansi, ������ scanf.

// v0.16
// ����

// v0.17
// printsc

// v0.18
// 

// v0.19

// v0.20
// ��������� ������: stm32f746 � stm32f303

// v0.21 (22 ������ 2018)
// ���������������� ��������� �����������: STM32F4xx

// v0.22
// ��������� ������: stm32h743

// ============================================================================================================================= //
// ========================================================= ��������� ========================================================= //
// ============================================================================================================================= //

// ==================== ����� ������������� ����������
#define UART_CPU_STM32F10x
//#define UART_CPU_STM32F3xx
//#define UART_CPU_STM32F4xx
//#define UART_CPU_STM32F7xx
//#define UART_CPU_STM32H7xx

// ==================== ������� ������������ ��� APB1, APB2 ����������.
/* Will be calculated later in UART_Init() */
//#define UART_CLK_APB1   72000000/2 // Hz
//#define UART_CLK_APB2   72000000 // Hz
#define UART_TIM_CLK_ML 2 //

// ==================== ������������ ���-�� ���� ��� ������ � ��������.
#define MAX_UART_DATA           300

// ==================== Enable UART
#define UART1_ENABLE
//#define UART2_ENABLE
//#define UART3_ENABLE
//#define UART4_ENABLE
//#define UART5_ENABLE
//#define UART6_ENABLE

// ==================== Enable RS485
//#define UART1_RS485
//#define UART2_RS485
//#define UART3_RS485
//#define UART4_RS485
//#define UART5_RS485
//#define UART6_RS485

// ========== Settings RS485 (Port, Pin)
#define UART1_DE_PORT   GPIOB
#define UART1_DE_PIN    GPIO_ODR_OD13

#define UART2_DE_PORT   GPIOA
#define UART2_DE_PIN    GPIO_ODR_OD10

#define UART3_DE_PORT   GPIOB
#define UART3_DE_PIN    GPIO_ODR_OD12

#define UART4_DE_PORT   GPIOB
#define UART4_DE_PIN    GPIO_ODR_OD1

#define UART5_DE_PORT   GPIOB
#define UART5_DE_PIN    GPIO_ODR_OD1

#define UART6_DE_PORT   GPIOG
#define UART6_DE_PIN    GPIO_ODR_OD10

// ============================================================================================================================= //
// ========================================================== Include ========================================================== //
// ============================================================================================================================= //
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#ifdef UART_CPU_STM32F10x
  #include "stm32f10x.h"
#endif

#ifdef UART_CPU_STM32F3xx
  #include "stm32f303xc.h"
#endif

#ifdef UART_CPU_STM32F4xx
  #include "stm32f407xx.h"
#endif

#ifdef UART_CPU_STM32F7xx
  #include "stm32f746xx.h"
#endif

#ifdef UART_CPU_STM32H7xx
  #include "stm32h743xx.h"
#endif

// ============================================================================================================================= //
// ============================================================================================================================= //
// ============================================================================================================================= //

#define UART_MODE_NORM          0
#define UART_MODE_PRINT         1

#define UART_DATA_8BITS         8 // 8 Data bits
#define UART_DATA_9BITS         9 // 9 Data bits

#define UART_STOP_05BIT         1 // 0.5 Stop bit
#define UART_STOP_1BIT          2 // 1 Stop bit
#define UART_STOP_15BIT         3 // 1.5 Stop bit
#define UART_STOP_2BITS         4 // 2 Stop bits

#define UART_NONE_PARITY        0 // 
#define UART_ODD_PARITY         1 // 
#define UART_EVEN_PARITY        2 // 

// ===============
#define UART_BUSY_FREE          1
#define UART_BUSY_SEND          0

#define UART_STATUS_FREE        0
#define UART_STATUS_RECV        1
#define UART_STATUS_ACCEPT      2

#define UART_TIM_ENABLE         1
#define UART_TIM_DISABLE        0

#define  UART_STATE_INITIALIZED			1
#define  UART_STATE_NOT_INIT_GPIO		2
//#define  UART_STATE_NOT_INIT_		3

struct uart_tx_rx_str
{
  unsigned char Data[MAX_UART_DATA];
  unsigned short Length;
  unsigned short Counter;
  unsigned char Status;
};

struct uart_tim_str
{
  unsigned short CNT; // ������� �������
  unsigned short CEN; //
	unsigned short TimeWord;
  unsigned short Timeout;
};

struct uart_gpio_str
{
	GPIO_TypeDef* GPIOx;
	unsigned long Pin;
};

struct uart_sett_str
{
	struct uart_gpio_str Tx;
	struct uart_gpio_str Rx;
	struct uart_gpio_str DE;
	unsigned long Speed;
	unsigned char DataBits;
	unsigned char StopBits;
	unsigned char Parity;
	unsigned char Mode;
};

struct uart_str
{
	//USART_TypeDef* USARTx;
	unsigned char State;

	//struct uart_sett_str Settings;

  struct uart_tx_rx_str Tx;
	// Tx.Data[MAX_UART_DATA]; // ������ ������
	// Tx.Length; // ���-�� ���� ������� ����� ��������
	// Tx.Counter; // �������, ����������� ������������ �����
	// Tx.Status; // ���� ���������: 1 - ��������, 0 - �������� !!

  struct uart_tx_rx_str Rx;
	// Rx.Data[MAX_UART_DATA]; // ������ ������
  // Rx.Length; // ���-�� �������� ����
  // Rx.Counter; // �������, ����������� �������� �����
  // Rx.Status; // 0 - �������� , 1 - ����� ����������� �������, 2 - ��� ������ ������� !!

  struct uart_tim_str TIM[2];	
};

// ============================================================================================================================= //
// ========================================================== ������� ========================================================== //
// ============================================================================================================================= //

//
void UART_Init(USART_TypeDef* USARTx, struct uart_str* UARTx, long Speed, char Data_bits, char Stop_bits, char parity, char mode);

//
unsigned int UART_Send_Interrupt(USART_TypeDef* USARTx, struct uart_str* UARTx, unsigned char *buffer, unsigned short count);
unsigned int UART_Receive(struct uart_str* UARTx, unsigned char *buffer, unsigned short buffer_size);

// 
//void prints(USART_TypeDef* USARTx, char *str);
// ������:
// ���������������� � uart.h ������ USART/UART (#define UART4_ENABLE)
// extern struct uart_str UART4STR; // ��������� ��� ����� ��������������
// char str[200]; // ��������� ��� ����� ��������������
// Init_UART(UART4, &UART4STR, 19200, UART_DATA_8BITS, UART_STOP_2BITS, UART_NONE_PARITY, UART_MODE_PRINT); // �� �������� �����
// sprintf(str, "prn = %i\n", 15); // ���������� � ����
// prints(UART4, str);

//char scansi(USART_TypeDef* USARTx, unsigned char *num);
//char scansf(USART_TypeDef* USARTx, char *str);

//
//void printsc(USART_TypeDef* USARTx, char *str, unsigned short cnt);
#endif
