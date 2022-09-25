/*
 * 2010-2022 Michal Demin
 *
 */

#ifndef FIFO_H_
#define FIFO_H_

#include "platform.h"

#define FIFO_EMPTY(x) ((((struct fifo_t *)x)->write == (x)->read))
#define FIFO_FULL(x)  (((((struct fifo_t *)x)->write + 1) % (x)->e_num) == (x)->read)

struct fifo_t {
	volatile uint32_t read;   // read pointer
	volatile uint32_t write;  // write pointer
	uint32_t e_num;  // number of elements
	uint32_t e_size; // element size
	void *buffer;
};

void fifo_init(struct fifo_t *b, void *buffer, uint32_t e_size, uint32_t e_num);
void fifo_reset(struct fifo_t *);

uint32_t fifo_empty(struct fifo_t *b);
uint32_t fifo_get_available(struct fifo_t *b);
uint32_t fifo_get_free_space(struct fifo_t *b);

void *fifo_get_read_addr(struct fifo_t *b);
void fifo_read_done(struct fifo_t *b);

void *fifo_get_write_addr(struct fifo_t *b);
void fifo_write_done(struct fifo_t *b);

uint32_t fifo_get_read_size_cont(struct fifo_t *b);
void fifo_read_done_size(struct fifo_t *b, uint32_t size);

void fifo_write_more(struct fifo_t *b, const void *buf, uint32_t cnt);

#endif
