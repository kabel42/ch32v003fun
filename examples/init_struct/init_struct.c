/* Small example showing how to use structs for controling GPIO pins */

#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

int main()
{
	// Enable GPIOs
	DYN_RCC_WRITE(APB2PCENR, (RCC_APB2PCENR_t){.IOPDEN = 1});
	// GPIO D0, D4 Push-Pull, D1/SWIO floating, default analog input
	DYN_GPIO_WRITE(GPIOD, CFGLR, (GPIO_CFGLR_t){.PIN0 = GPIO_CFGLR_OUT_10Mhz_PP, .PIN1 = GPIO_CFGLR_IN_FLOAT, .PIN4 = GPIO_CFGLR_OUT_10Mhz_PP}); // (GPIO_CFGLR_t) is optional but helps vscode with completion

	DYN_RCC_WRITE(CFGR0, (RCC_CFGR0_t){.SW = 0, .HPRE = 0, .ADCPRE = 0, .PLLSRC = 0, .MCO = 0});
	DYN_RCC_WRITE(CTLR, (RCC_CTLR_t){.HSION = 1, .HSITRIM = HSITRIM_DEFAULT, .HSEON = 0, .HSEBYP = 0, .CSSON = 0, .PLLON = 1});
	FLASH->ACTLR = FLASH_ACTLR_LATENCY_1; // >= 24MHz -> 1 Cycle Latency
	DYN_RCC_WRITE(INTR, (RCC_INTR_t){.HSIRDYIE = 0, .HSERDYIE = 0, .PLLRDYIE = 0, .LSIRDYC = 1, .HSIRDYC = 1, .HSERDYC = 1, .CSSC = 1});

	// Wait till PLL is ready
	while ((RCC->CTLR & RCC_PLLRDY) == 0);

	// Select PLL as system clock source
	DYN_RCC_MOD(CFGR0, SW, RCC_SW_PLL);
	// Wait till PLL is used as system clock source
	while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08);

	DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t){.BR0 = 1, .BR4 = 1});

	SetupDebugPrintf();

	Delay_Ms(100);

	printf("UP\n");

	while (1)
	{
		// Turn D0 on and D4 off at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t){.BS0 = 1, .BR4 = 1});
		Delay_Ms(100);

		// Turn D0 off and D4 on at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t){.BR0 = 1, .BS4 = 1});
		Delay_Ms(100);
	}
}
