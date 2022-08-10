#ifndef DE_h_
#define DE_h_

#include "FreeRTOS.h"
#include "semphr.h"

void de_init(void);
void de_commit(void);

void de_layer_set(void *fb, void *fb1);
void *de_layer_get_fb(void);
int de_layer_swap_done(void);
void de_layer_swap(void);
void de_layer_register_semaphore(SemaphoreHandle_t s);

void de_int_vblank(void);

#endif
