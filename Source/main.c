#include "main.h"

extern struct uart_str UART1STR;

unsigned int LengthRx = 0;
unsigned char BufferRx[MINMEA_MAX_SENTENCE_LENGTH*5];	// up to 5 sentences are needed

struct minmea_sentence_rmc RMCframe;
struct minmea_sentence_vtg VTGframe;
struct minmea_sentence_gga GGAframe;
struct minmea_sentence_gsa GSAframe;
struct minmea_sentence_gsv GSVframe;

char TimeStr[9];
char SatsQTY;

uint32_t Comm_WDT;


void main(void)
{
	asm("CPSID i");
	Sys_Init();
	IO_Init();
#ifdef INVOLVE_DISPLAY
	I2C1_Init();
	SSD1306_Init();
#endif
	UART_Init(USART1, &UART1STR, 9600, UART_DATA_8BITS, UART_STOP_1BIT, UART_NONE_PARITY, UART_MODE_NORM);
	Start_Blink();
	asm("CPSIE i");

	while(1)
	{
		LengthRx = UART_Receive(&UART1STR, BufferRx, sizeof(BufferRx));
		if ((LengthRx > 10) && ((strncmp((char const *)BufferRx, "$GNGGA", 6)) == 0)) // there is a data in the buffer and GNGGA - first sentence in the frame (E108 GN02D-default)
			//		if ((LengthRx > 10) && ((strncmp((char const *)BufferRx, "$GPRMC", 6)) == 0)) // there is a data in the buffer and GPRMC - first sentence in the frame (NEO-6M-default)
		{
			Comm_WDT = 5000000;
			GPS_parseVTG(BufferRx, sizeof(BufferRx), &VTGframe);
			GPS_parseRMC(BufferRx, sizeof(BufferRx), &RMCframe);
			GPS_parseGGA(BufferRx, sizeof(BufferRx), &GGAframe);
			GPS_parseGSA(BufferRx, sizeof(BufferRx), &GSAframe);
			GPS_parseGSV(BufferRx, sizeof(BufferRx), &GSVframe);
#ifdef INVOLVE_DISPLAY
			if (GGAframe.time.hours == -1)	// no fix data from sats
			{
				SSD1306_Fill(SSD1306_COLOR_BLACK);
				SSD1306_GotoXY(0, 0);
				SSD1306_Puts("SATELLITES QTY:", &Font_7x10, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY(0, 32-11);
				sprintf(&SatsQTY, "%.2i", GSVframe.total_sats);
				SSD1306_Puts(&SatsQTY, &Font_7x10, SSD1306_COLOR_WHITE);
				SSD1306_UpdateScreen();
			}
			else	// data fixed
			{
				SSD1306_Fill(SSD1306_COLOR_BLACK);
				/* forming the "time" string */
				if (GGAframe.time.hours < 20) sprintf(TimeStr, "%.2i", (GGAframe.time.hours) + TIME_CORRECTION_HOUR);
				else sprintf(TimeStr, "%.2i", (GGAframe.time.hours) + TIME_CORRECTION_HOUR - 24);
				memset(&TimeStr[2], ':', 1);
				sprintf(TimeStr+3, "%.2i", GGAframe.time.minutes);
				memset(&TimeStr[5], ':', 1);
				sprintf(TimeStr+6, "%.2i", GGAframe.time.seconds);
				/* display */
				// time
				SSD1306_GotoXY(0, 0);
				SSD1306_Puts("GPS TIME:", &Font_7x10, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY(0, 32-11);
				SSD1306_Puts(TimeStr, &Font_7x10, SSD1306_COLOR_WHITE);
				// border
				SSD1306_DrawLine(67, 0, 67, 32, SSD1306_COLOR_WHITE);
				SSD1306_DrawLine(70, 0, 70, 32, SSD1306_COLOR_WHITE);
				// sats
				SSD1306_GotoXY(75, 0);
				SSD1306_Puts("SATS:", &Font_7x10, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY(75, 32-11);
				sprintf(&SatsQTY, "%.2i", GSVframe.total_sats);
				SSD1306_Puts(&SatsQTY, &Font_7x10, SSD1306_COLOR_WHITE);
				SSD1306_UpdateScreen();
			}
#endif
		}
		else if (Comm_WDT > 0) Comm_WDT--;
#ifdef INVOLVE_DISPLAY
		else
		{
			SSD1306_Fill(SSD1306_COLOR_BLACK);
			SSD1306_GotoXY(0, 0);
			SSD1306_Puts("UART ERR", &Font_11x18, SSD1306_COLOR_WHITE);
			SSD1306_UpdateScreen();
		}
#endif
	}
}

/**
*	@brief Custom Functions
*/
void IO_Init(void)
{
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);

	//----- USART1 -----//
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
	/* PA9 - TX */
	SET_BIT(GPIOA->CRH, GPIO_CRH_MODE9);	// Output mode, max speed 50 MHz
	MODIFY_REG(GPIOA->CRH,GPIO_CRH_CNF9, GPIO_CRH_CNF9_1);	// Alternate function output Push-pull
	/* PA10 - RX */
	CLEAR_BIT(GPIOA->CRH, GPIO_CRH_MODE10);	// Input mode (reset state)
	MODIFY_REG(GPIOA->CRH,GPIO_CRH_CNF10, GPIO_CRH_CNF10_1);	// Input with pull-up / pull-down
	SET_BIT(GPIOA->BSRR, 1 << 10);	// PA10 pull-up
	//----- USER LED -----//
	/* PC13 */
	SET_BIT(GPIOC->CRH, GPIO_CRH_MODE13);	// Output mode, max speed 50 MHz
	MODIFY_REG(GPIOC->CRH,GPIO_CRH_CNF13, GPIO_CRH_CNF13_0);	// General purpose output Open-drain
}

void Start_Blink(void)
{
	LED_ON;
	_delay_ms(200);
	LED_OFF;
	_delay_ms(200);
	LED_ON;
	_delay_ms(200);
	LED_OFF;
	_delay_ms(200);
	LED_ON;
	_delay_ms(200);
	LED_OFF;
	_delay_ms(200);
}