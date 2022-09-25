#ifndef MEMMGR_h_
#define MEMMGR_h_

void memmgr_init(void);

void *dma_malloc(size_t size);
void *dma_memalign(size_t align, size_t size);
void *dma_realloc(void * ptr, size_t size);
void dma_free(void * ptr);

/*
void *mm_malloc(size_t size);
void *mm_memalign(size_t align, size_t size);
void *mm_realloc(void * ptr, size_t size);
void *mm_calloc(size_t nmemb, size_t size);
void mm_free(void * ptr);
*/

#endif

