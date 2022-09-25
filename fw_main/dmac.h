#ifndef DMAC_h_
#define DMAC_h_

#include "platform.h"

#define DMAC_NO_LINK ((struct dmac_desc_t *)0xFFFFF800)

struct dmac_desc_t {
	uint32_t config;
	void *src_addr;
	void *dst_addr;
	uint32_t nbytes;
	uint32_t parameter;
	struct dmac_desc_t *next;
} __attribute__((packed));

typedef void (*dmac_handler_t)(uint8_t, void *);

void dmac_init(void);
void dmac_set_handler(uint8_t ch, dmac_handler_t fnc, void *arg);
void dmac_start(uint8_t ch, struct dmac_desc_t *desc);
void dmac_enable_irq(uint8_t ch, uint8_t flag);

#endif

