#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	// Values lifted from the EVT.  There is little to no documentation on what this does.
	RCC->CFGR0 = RCC_HPRE_DIV1 | RCC_PLLSRC_HSI_Mul2;
	RCC->CTLR  = RCC_HSION | RCC_HSEON | RCC_PLLON | ((HSITRIM_DEFAULT) << 3); // Use HSI, but enable PLL.
	FLASH->ACTLR = FLASH_ACTLR_LATENCY_1;                  // 1 Cycle Latency
	RCC->INTR  = 0x009F0000;                               // Clear PLL, CSSC, HSE, HSI and LSI ready flags.

	// From SetSysClockTo_48MHZ_HSI
	while((RCC->CTLR & RCC_PLLRDY) == 0);                                      // Wait till PLL is ready
	RCC->CFGR0 = ( RCC->CFGR0 & ((uint32_t)~(RCC_SW))) | (uint32_t)RCC_SW_PLL; // Select PLL as system clock source
	while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08);                // Wait till PLL is used as system clock source

    SetupDebugPrintf();
	//WaitForDebuggerToAttach();

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// PC4 is T1CH4, 50MHz Output PP CNF = 10: Mux PP, MODE = 11: Out 50MHz
	GPIOC->CFGLR &= ~(GPIO_CFGLR_MODE4 | GPIO_CFGLR_CNF4);
	GPIOC->CFGLR |= GPIO_CFGLR_CNF4_1 | GPIO_CFGLR_MODE4_0 | GPIO_CFGLR_MODE4_1;

	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_PLL;

	while(1)
	{
	}
}
