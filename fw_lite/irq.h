#ifndef IRQ_h_
#define IRQ_h_

#include "platform.h"

typedef void (*inthandler_t)(uint32_t, void *);

void irq_init(void);
void irq_set_handler(uint32_t irq, inthandler_t fnc, void *arg);
void irq_set_prio(uint32_t irq, uint8_t prio);
void irq_set_enable(uint32_t irq, uint8_t state);

#endif

