// timer for FreeRTOS using alwinner internal timer
#include "platform.h"

#include "ccu.h"
#include "irq.h"

#include "timer.h"

#include "FreeRTOS.h"

#define TMR0_MODE       BV(7)  // 1 - single shot, 0 periodic
#define TMR0_CLK_PRES_shift 4  // 2**n
#define TMR0_CLK_SRC_shift  2  // 0 - LOsC, 1 OSC24M
#define TMR0_RELOAD     BV(1)
#define TMR0_EN         BV(0)

#define TIMER_TEST 0

#if TIMER_TEST
void tim1(uint32_t aa, void *arg)
{
	uart_printf("timer irq\n");
	vClearTickInterrupt();
}
#endif

void vConfigureTickInterrupt(void)
{
	uint32_t clk = ccu_clk_hosc_get();

//	uart_printf("timer: init\n");

#if TIMER_TEST
	TIMER->TMR0_INTV_VALUE_REG = clk / 2 ; // 1sec period test
#else
	TIMER->TMR0_INTV_VALUE_REG = clk / 2 / configTICK_RATE_HZ;
#endif
	TIMER->TMR0_CTRL_REG = /*BV(7) |*/ (1 << 4) | (1 << 2); // periodic, 2prescale, osc24m

	// reload
	TIMER->TMR0_CTRL_REG |= BV(1);
	while(TIMER->TMR0_CTRL_REG & BV(1));

	TIMER->TMR0_CTRL_REG |= BV(0); // enable
																 //
	TIMER->TMR_IRQ_STA_REG = BV(0); // reset pending
	TIMER->TMR_IRQ_EN_REG |= BV(0); // enable interrupt

#if TIMER_TEST
	irq_set_handler(TIMER0_IRQn, tim1, NULL );
	irq_set_prio(TIMER0_IRQn, 0x7c);
	irq_set_enable(TIMER0_IRQn, 1);
#else
	// install FreeRTOS hook
	// This tick interrupt must run at the lowest priority.
	irq_set_handler(TIMER0_IRQn, (inthandler_t) FreeRTOS_Tick_Handler, NULL );
	irq_set_prio(TIMER0_IRQn, portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );
	irq_set_enable(TIMER0_IRQn, 1);
#endif
}

void vClearTickInterrupt(void)
{
//	uart_printf("timer: iclr\n");
	TIMER->TMR_IRQ_STA_REG = BV(0);
}
