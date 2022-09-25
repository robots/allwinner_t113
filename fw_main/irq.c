#include "platform.h"
#include <string.h>

#include "uart.h"

#include "irq.h"

#define IRQ_MAX 1020

struct irq_tab_t {
	inthandler_t fnc;
	void *arg;
};

struct irq_tab_t irq_table[IRQ_MAX];


struct arm_regs_t {
	uint32_t esp;
	uint32_t cpsr;
	uint32_t r[13];
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
};

static void default_handler(uint32_t irq, void *arg);

void irq_init(void)
{
	extern unsigned long _vector;
//	uart_printf("irq: init\n\r");
	__set_VBAR(&_vector);
	__set_MVBAR(&_vector);
	__set_SCTLR(__get_SCTLR() & ~SCTLR_V_Msk);
  
	// set default irq handler
	for (uint32_t i = 0; i < IRQ_MAX; i++) {
		irq_table[i].fnc = default_handler;
		irq_table[i].arg = 0;
	}

	// enable distributor and percpu iface
  GIC_Enable();

	// enable all SGIs
	for (uint32_t i = 0; i < 16; i++) {
		GIC_EnableIRQ(i);
	}

#if 0 // test 
 // enable all irq
	for (uint32_t i = 0; i < 1020; i++) {
		if ((i > 197) && (i < 200)) continue;

		GIC_EnableIRQ(i);
	}
#endif
//	GICDistributor->CTLR |= 3;
//	GICInterface->CTLR |= BV(0) | BV(1) | BV(3);

  GIC_SetInterfacePriorityMask(0xffU);
	uart_printf("irq: init done GICD->CTRL=%08x GICC->CTRL = %08x\n\r", GICDistributor->CTLR, GICInterface->CTLR);
}

void irq_set_handler(uint32_t irq, inthandler_t fnc, void *arg)
{
	irq_table[irq].fnc = fnc;
	irq_table[irq].arg = arg;
}

void irq_set_enable(uint32_t irq, uint8_t state)
{
	if (state) {
		GIC_EnableIRQ(irq);
	} else {
		GIC_DisableIRQ(irq);
	}
}

void irq_set_prio(uint32_t irq, uint8_t prio)
{
//	uart_printf("irq: set irq %d prio %d \n", irq, prio);
	GIC_SetPriority(irq, prio);
}

//void vApplicationFPUSafeIRQHandler( uint32_t ulICCIAR )
void vApplicationIRQHandler( uint32_t ulICCIAR )
{
	/* Re-enable interrupts. */
	__asm ( "cpsie i" );

//	uart_printf("irq: handler %d\n", ulICCIAR);

	/* The ID of the interrupt is obtained by bitwise anding the ICCIAR value	with 0x3FF. */
	uint32_t irq = ulICCIAR & 0x3FFUL;
	if(irq < ARRAY_SIZE(irq_table))
	{
		if (irq_table[irq].fnc) {
			irq_table[irq].fnc(irq_table[irq].arg);
		}
	}
}

void arm32_do_undefined_instruction(struct arm_regs_t *regs)
{
	(void)regs;
	uart_printf("irq: undefi\n\r");
	while(1);
}

void arm32_do_software_interrupt(struct arm_regs_t *regs)
{
	(void)regs;
	uart_printf("irq: swi\n\r");
	while(1);
}

void arm32_do_prefetch_abort(struct arm_regs_t *regs)
{
	(void)regs;
	uart_printf("irq: p abrt lr=%08x pc=%08x\n\r", regs->lr, regs->pc);
	while(1);
}

void arm32_do_data_abort(struct arm_regs_t *regs)
{
	(void)regs;
	uart_printf("irq: d abrt lr=%08x pc=%08x\n\r", regs->lr, regs->pc);
	while(1);
}

void arm32_do_irq(struct arm_regs_t *regs)
{
	(void)regs;
	uint32_t iar = GIC_AcknowledgePending();
	uart_printf("irq: irq iar = %08x irq =  %d\n\r", iar, iar & 0x3ff);
	uint32_t irq = iar & 0x3FFUL;
	if(irq < ARRAY_SIZE(irq_table))
	{
		if (irq_table[irq].fnc) {
			irq_table[irq].fnc(irq_table[irq].arg);
		}
	}
	GIC_EndInterrupt(iar);
}

void arm32_do_fiq(struct arm_regs_t *regs)
{
	(void)regs;
	uint32_t iar = GIC_AcknowledgePending();
	uart_printf("irq: fiq iar = %08x irq =  %d\n\r", iar, iar & 0x3ff);
	uint32_t irq = iar & 0x3FFUL;
	if(irq < ARRAY_SIZE(irq_table))
	{
		if (irq_table[irq].fnc) {
			irq_table[irq].fnc(irq_table[irq].arg);
		}
	}
	GIC_EndInterrupt(iar);
}

void default_handler(uint32_t irq, void *arg)
{
	(void)arg;
	uart_printf("\r\nirq: unexpected irq %d\n\r", irq);
	while(1);
}
