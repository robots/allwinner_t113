#include "platform.h"
#include "tlsf.h"
#include <string.h>
#include "uart.h"

#include "memmgr.h"

static tlsf_t dma_pool = NULL;
//static tlsf_t heap_pool = NULL;

void *dma_malloc(size_t size)
{
	return tlsf_malloc(dma_pool, size);
}

void *dma_memalign(size_t align, size_t size)
{
	return tlsf_memalign(dma_pool, align, size);
}

void *dma_realloc(void * ptr, size_t size)
{
	return tlsf_realloc(dma_pool, ptr, size);
}

void dma_free(void * ptr)
{
	tlsf_free(dma_pool, ptr);
}

/*
void *mm_malloc(size_t size)
{
	return tlsf_malloc(heap_pool, size);
}

void *mm_memalign(size_t align, size_t size)
{
	return tlsf_memalign(heap_pool, align, size);
}

void *mm_realloc(void * ptr, size_t size)
{
	return tlsf_realloc(heap_pool, ptr, size);
}

void *mm_calloc(size_t nmemb, size_t size)
{
	void * ptr;

	if((ptr = mm_malloc(nmemb * size)))
		memset(ptr, 0, nmemb * size);

	return ptr;
}

void mm_free(void * ptr)
{
	tlsf_free(heap_pool, ptr);
}

extern unsigned char __heap_start;
extern unsigned char __heap_end;
*/
extern unsigned char __dma_start;
extern unsigned char __dma_end;

void memmgr_init(void)
{
/*
	size_t s = (size_t)(&__heap_end - &__heap_start);

	uart_printf("heap: creating heap @ %08x size %d\n", &__heap_start, s);
	heap_pool = tlsf_create_with_pool((void *)&__heap_start, s);
*/

	size_t s = (size_t)(&__dma_end - &__dma_start);
	uart_printf("heap: creating dma pool @ %08x size %d\n", &__dma_start, s);
	dma_pool = tlsf_create_with_pool((void *)&__dma_start, s);
}
