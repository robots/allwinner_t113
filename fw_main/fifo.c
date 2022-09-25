/*
 * 2014-2022 Michal Demin
 *
 * Lock-free implementation of fifo.
 *
 * Write will never modify read ptr, and vice-versa.
 * If read/write ptr changes during write/read it will
 * create more space for write/read operation.
 *
 * Fifo will hold up to e_size-1 elements. (one is always wasted)
 *
 */

#include "platform.h"

#include <string.h>

#include "fifo.h"

/*
 * buffer size needs to be "e_size * e_num"
 */
void fifo_init(struct fifo_t *b, void *buffer, uint32_t e_size, uint32_t e_num)
{
	b->write = 0;
	b->read = 0;
	b->e_num = e_num;
	b->e_size = e_size;
	b->buffer = buffer;
}

void fifo_reset(struct fifo_t *b)
{
	b->write = 0;
	b->read = 0;
}

uint32_t fifo_empty(struct fifo_t *b)
{
	if (FIFO_EMPTY(b)) {
		return 1;
	}

	return 0;
}

uint32_t fifo_get_available(struct fifo_t *b)
{
	uint32_t out;

	uint32_t r = b->read;
	uint32_t w = b->write;

	if (r > w) {
		out = r - w;
		out = b->e_num - out;
	} else {
		out = w - r;
	}

	return out;
}

uint32_t fifo_get_free_space(struct fifo_t *b)
{
	uint32_t out;

	out = fifo_get_available(b);
	out = b->e_num - out - 1;

	return out;
}


void *fifo_get_read_addr(struct fifo_t *b)
{
	return (void *) ((uint32_t)b->buffer + b->read * b->e_size);
}

void *fifo_get_write_addr(struct fifo_t *b)
{
	return (void *) ((uint32_t)b->buffer + b->write * b->e_size);
}

void fifo_read_done(struct fifo_t *b)
{
	// if there is something more to read -> advance
	if (!FIFO_EMPTY(b)) {
		size_t r = b->read;
		r++;
		r %= b->e_num;
		b->read = r;
	}
}

void fifo_write_done(struct fifo_t *b)
{
	if (FIFO_FULL(b)) {
		return;
	}

	uint32_t w = b->write;
	w ++;
	w %= b->e_num;
	b->write = w;

}

// returns length of continuous buffer
uint32_t fifo_get_read_size_cont(struct fifo_t *b)
{
	uint32_t r = b->read;
	uint32_t w = b->write;

	if (w < r) {
		return b->e_num - r;
	}
	
	if (w > r) {
		return w - r;
	}

	return 0;
}

void fifo_read_done_size(struct fifo_t *b, uint32_t size)
{
	size_t r = b->read;
	r += size;
	r %= b->e_num;
	b->read = r;
}

// assumes that:
// - buf/cnt will fit into the fifo
// - noone else tries to write fifo
//
void fifo_write_more(struct fifo_t *b, const void *buf, uint32_t cnt)
{
#if 0
	uint32_t i;
	void *ptr;

	for (i = 0; i < cnt; i++) {
		ptr = FIFO_GetWriteAddr(b);

		if (b->e_size == 1) {
			(uint8_t *ptr)[i] = (uint8_t *buf)[i];
		} else if (b->e_size == 2) {
			(uint16_t *ptr)[i] = (uint16_t *buf)[i];
		} else if (b->e_size == 4) {
			(uint32_t *ptr)[i] = (uint32_t *buf)[i];
		} else {
			memcpy(ptr, buf+b->e_size*i, b->e_size);
		}

		FIFO_WriteDone(b);
	}
#else
	uint32_t r = b->read;
	uint32_t w = b->write;

	if ((cnt > 0) && (w >= r)) {
		uint32_t c = MIN(cnt, b->e_num - w);

		memcpy((void *)((uint32_t)b->buffer + w * b->e_size), buf, c * b->e_size);

		w += c;
		w %= b->e_num;
		b->write = w;

		cnt -= c;
		buf = (void*)((uint32_t)buf + c * b->e_size);
	}

	if ((cnt > 0) && (r >= w)) {
		uint32_t c = MIN(cnt, r - w - 1);

		memcpy((void *)((uint32_t)b->buffer + b->write * b->e_size), buf, c * b->e_size);

		w += c;
		w %= b->e_num;
		b->write = w;

		cnt -= c;
		buf = (void*)((uint32_t)buf + c * b->e_size);
	}
#endif
}
