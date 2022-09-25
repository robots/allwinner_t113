/* dmac interrupt demultiplexer */

#include "platform.h"

#include "irq.h"

#include "dmac.h"

#include "FreeRTOS.h"


#define DMAC_CHANNELS 16

struct {
	dmac_handler_t fnc;
	void *arg;
} dmac_handlers[DMAC_CHANNELS];

static void dmac_int_handler(void *arg);

void dmac_init(void)
{
	irq_set_handler(DMAC_NS_IRQn, dmac_int_handler, NULL );
	irq_set_prio(DMAC_NS_IRQn, configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );
	irq_set_enable(DMAC_NS_IRQn, 1);
}

void dmac_set_handler(uint8_t ch, dmac_handler_t fnc, void *arg)
{
	if (ch >= DMAC_CHANNELS) return;

	dmac_handlers[ch].fnc = fnc;
	dmac_handlers[ch].arg = arg;
}

void dmac_start(uint8_t ch, struct dmac_desc_t *desc)
{
	if ((uint32_t)desc & 3) {
		uart_printf("dmac unaligned\n");
		while(1);
	}

	DMAC->CH[ch].DMAC_EN_REGN = 0;
	DMAC->CH[ch].DMAC_DESC_ADDR_REGN = (uint32_t)desc;
	DMAC->CH[ch].DMAC_PAU_REGN = 0;
	DMAC->CH[ch].DMAC_EN_REGN = 1;
}

void dmac_enable_irq(uint8_t ch, uint8_t flag)
{
	flag &= 7;

	if (ch > 8) {
		ch -= 8;

		uint32_t reg = DMAC->DMAC_IRQ_EN_REG1;
		reg &= 0x07 << (4 * ch);
		reg |= flag << (4 * ch);
		DMAC->DMAC_IRQ_EN_REG1 |= reg;
	} else {
		uint32_t reg = DMAC->DMAC_IRQ_EN_REG0;
		reg &= 0x07 << (4 * ch);
		reg |= flag << (4 * ch);
		DMAC->DMAC_IRQ_EN_REG0 |= reg;
	}
}

static void dmac_int_handler(void *arg)
{
	uint32_t pend0 = DMAC->DMAC_IRQ_PEND_REG0 & DMAC->DMAC_IRQ_EN_REG0;
	uint32_t pend1 = DMAC->DMAC_IRQ_PEND_REG1 & DMAC->DMAC_IRQ_EN_REG1;

	for (uint32_t i = 0; i < 7; i++) {
		uint32_t mask = 0x7 << (i*4);
		if (pend0 & mask) {
			if (dmac_handlers[i].fnc) {
				dmac_handlers[i].fnc(i, dmac_handlers[i].arg);
			}
		}
	}

	for (uint32_t i = 0; i < 7; i++) {
		uint32_t mask = 0x7 << (i*4);
		if (pend1 & mask) {
			if (dmac_handlers[8+i].fnc) {
				dmac_handlers[8+i].fnc(8+i, dmac_handlers[i].arg);
			}
		}
	}

	DMAC->DMAC_IRQ_PEND_REG0 = pend0;
	DMAC->DMAC_IRQ_PEND_REG1 = pend1;
}
