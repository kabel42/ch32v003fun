/* Small example showing how to capture timer values on gpio edges */

#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

// static const size_t queuelen = 16;
#define queuelen 16
volatile uint32_t captureVals[queuelen];
volatile size_t read = 0;
volatile size_t write = 0;

void TIM1_CC_IRQHandler(void) __attribute__((interrupt));
void TIM1_CC_IRQHandler(void)
{
	// capture
	if (DYN_TIM_READ(TIM1, INTFR).CC1IF)
	{
		// get capture
		captureVals[write++] = 0x00010000 | TIM1->CH1CVR; // capture valur
		if (write == queuelen)
		{
			write = 0;
		}
		// overflow
		if (DYN_TIM_READ(TIM1, INTFR).CC1OF)
		{
			// clear
			TIM1->INTFR = ~(TIM_CC1OF); // cleard by writing 0
			printf("OF1\n");
		}
	}
	else if (DYN_TIM_READ(TIM1, INTFR).CC2IF)
	{
		// get capture
		captureVals[write++] = TIM1->CH2CVR; // capture valur
		if (write == queuelen)
		{
			write = 0;
		}
		// overflow
		if (DYN_TIM_READ(TIM1, INTFR).CC2OF)
		{
			// clear
			TIM1->INTFR = ~(TIM_CC2OF);  // cleard by writing 0
			printf("OF0\n");
		}
	}
	else
	{
		printf("badtrigger\n");
	}
}

int main()
{
	SystemInit48HSI();
	/*DYN_RCC_WRITE(CFGR0, (RCC_CFGR0_t) {.SW = 0, .HPRE = 0, .ADCPRE = 0, .PLLSRC = 0, .MCO = 0});
	DYN_RCC_WRITE(CTLR, (RCC_CTLR_t) {.HSION = 1, .HSITRIM = HSITRIM_DEFAULT, .HSEON = 0, .HSEBYP = 0, .CSSON = 0, .PLLON = 1});
	FLASH->ACTLR = FLASH_ACTLR_LATENCY_1;                  // >= 24MHz -> 1 Cycle Latency
	DYN_RCC_WRITE(INTR, (RCC_INTR_t){.HSIRDYIE=0,.HSERDYIE=0,.PLLRDYIE=0,.LSIRDYC=1,.HSIRDYC=1,.HSERDYC=1,.CSSC=1});
	// Wait till PLL is ready
	while(DYN_RCC_READ(CTLR).PLLRDY == 0){}
	// Select PLL as system clock source
	DYN_RCC_MOD(CFGR0, SW, RCC_SW_PLL);
	// Wait till PLL is used as system clock source
	while(DYN_RCC_READ(CFGR0).SWS != 2){}
	*/

	SetupDebugPrintf();

	Delay_Ms(100);

	printf("UP\n");

	// Enable peripherals
	DYN_RCC_WRITE(APB2PCENR, (RCC_APB2PCENR_t){.AFIOEN = 0, .IOPDEN = 1, .TIM1EN = 1}); // default disabled
	DYN_RCC_WRITE(APB1PCENR, (RCC_APB1PCENR_t){.TIM2EN = 0});

	DYN_RCC_WRITE(APB2PRSTR, (RCC_APB2PRSTR_t){.TIM1RST = 1});
	DYN_RCC_WRITE(APB2PRSTR, (RCC_APB2PRSTR_t){.TIM1RST = 0});

	// GPIO D0, D4 Push-Pull LEDs, D1/SWIO floating, D2 Capture Input(T1CH1), default analog input
	DYN_GPIO_WRITE(GPIOD, CFGLR, (GPIO_CFGLR_t){
									 // (GPIO_CFGLR_t) is optional but helps vscode with completion
									 .PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
									 .PIN1 = GPIO_CFGLR_IN_FLOAT,
									 .PIN2 = GPIO_CFGLR_IN_FLOAT,
									 .PIN4 = GPIO_CFGLR_OUT_10Mhz_PP,
								 });

	TIM1->ATRLR = 0xffff;
	TIM1->PSC = 47; // 48MHz/(47+1) -> 1µs resolution

	DYN_TIM_WRITE(TIM1, CHCTLR1, (TIM_CHCTLR1_t){.chctlr1_input_bits = {
													 .IC1F = 0000, .IC1PSC = 0,
													 .CC1S = 0b01, // input from PD2(T1CH1)
													 .IC2F = 0000, .IC2PSC = 0,
													 .CC2S = 0b10, // input from PD2(T1CH1)
												 }});
	DYN_TIM_WRITE(TIM1, CCER, (TIM_CCER_t){
								  .CC1E = 1,
								  .CC1P = 0, // CCxP polarity, 0 -> rising edge, 1 -> falling edge, ??? -> both edges
								  .CC2E = 1,
								  .CC2P = 1, // falling
							  });
	DYN_TIM_WRITE(TIM1, CTLR1, (TIM_CTLR1_t){
								   .CEN = 1,
							   });

	NVIC_EnableIRQ(TIM1_CC_IRQn);
	DYN_TIM_WRITE(TIM1, DMAINTENR, (TIM_DMAINTENR_t){.CC1IE = 1, .CC2IE = 1});
	//__enable_irq();

	while (1)
	{
		if (read != write)
		{
			uint32_t val = captureVals[read++];
			printf("capture %d %u\n", val>>16, val&0xFFFF);
			if (read == queuelen)
			{
				read = 0;
			}
		}

		// Turn D0 on and D4 off at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t){.BS0 = 1, .BR4 = 1});
		Delay_Ms(1);

		// Turn D0 off and D4 on at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t){.BR0 = 1, .BS4 = 1});
		Delay_Ms(1);
	}
}
