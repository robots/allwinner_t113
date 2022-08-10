#include "platform.h"

#include "uart.h"
#include "irq.h"

#include "arm_timer.h"

#define ARM_GT_CTL_ENABLE  0x01
#define ARM_GT_CTL_IMASK   0x02
#define ARM_GT_CTL_ISTATUS 0x04


#define TIMER_PERIOD  1

static uint32_t calc_period(uint32_t p)
{
	uint32_t c = __get_CNTFRQ();

	return c / p;
}

static void arm_timer_start(uint32_t t)
{
	uint32_t ctl;

	// set new downcount value
	__set_CNTP_TVAL(t);

	// reenable timer
	ctl = __get_CNTP_CTL();
	ctl &= ~ARM_GT_CTL_IMASK; 
	ctl |= ARM_GT_CTL_ENABLE;
	__set_CNTP_CTL(ctl);

	uart_printf("arm_timer: ctl = %08x tval = %08x frq = %08x\n", __get_CNTP_CTL(), __get_CNTP_TVAL(), __get_CNTFRQ());
}

static void arm_timer_stop(void)
{
	uint32_t ctl;

	// mask interrupt output
	ctl = __get_CNTP_CTL();
	ctl |= ARM_GT_CTL_IMASK; 
	ctl &= ~ARM_GT_CTL_ENABLE;
	__set_CNTP_CTL(ctl);
}

void tim(uint32_t aa, void *arg)
{
	arm_timer_stop();
	arm_timer_start(24000000);

	uart_printf("arm_timer %d %08x\n", (uint32_t)arg, aa);
}


void arm_timer_init(void)
{
	irq_set_handler(NonSecurePhysicalTimer_IRQn, tim, 1 );
	irq_set_prio(NonSecurePhysicalTimer_IRQn, 0x7c);
	irq_set_enable(NonSecurePhysicalTimer_IRQn, 1);

	irq_set_handler(SecurePhysicalTimer_IRQn, tim, 2 );
	irq_set_prio(SecurePhysicalTimer_IRQn, 0x7c);
	irq_set_enable(SecurePhysicalTimer_IRQn, 1);

	irq_set_handler(HypervisorTimer_IRQn, tim, 3 );
	irq_set_prio(HypervisorTimer_IRQn, 0x7c);
	irq_set_enable(HypervisorTimer_IRQn, 1);

	irq_set_handler(VirtualTimer_IRQn, tim, 4 );
	irq_set_prio(VirtualTimer_IRQn, 0x7c);
	irq_set_enable(VirtualTimer_IRQn, 1);

	arm_timer_start(24000000);
}

