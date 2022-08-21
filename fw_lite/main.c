#include "platform.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "led.h"
#include "irq.h"
#include "mmu.h"
#include "ccu.h"
#include "dmac.h"
#include "tcon_lcd.h"
#include "de.h"
#include "arm_timer.h"
#include "random.h"

#include "syscalls.h"

#include "memmgr.h"

#include "gr.h"

#include "uart.h"
#include "usb_task.h"


#define STACK_SIZE 200
TaskHandle_t task_blinky_handle;
TaskHandle_t task_gr_handle;

void task_blinky(void *arg)
{
	(void)arg;
	uint32_t state = 1;

	printf("blinky\n");

	while(1)
	{
		led_set(0, state);
		state = !state;
		vTaskDelay(250);
	}
}

void do_lcd(void)
{
	//uart_printf("pll video0(x4): %d\n", ccu_video0_x4_clk_get());

	void *fb = dma_memalign(128, 800*480*4);
	void *fb1 = dma_memalign(128, 800*480*4);

	printf("fb addr: %p and %p\n", fb, fb1);

	gr_fill(fb, 0xff000000);
	gr_fill(fb1, 0xff000000);
//	gr_fill(fb, 0xff333333);
	gr_draw_pixel(fb, 100, 100, 0xffff0000);
	gr_draw_pixel(fb, 101, 101, 0xffff0000);
	gr_draw_pixel(fb, 101, 100, 0xffff0000);
	gr_draw_pixel(fb, 100, 101, 0xffff0000);
	gr_draw_line(fb, 0, 0, 800-1, 480-1, 0xff00ff00);
	gr_draw_line(fb, 800-1, 0, 0, 480-1, 0xffff0000);

	tcon_lcd_init();
	tcon_lcd_enable();
	led_set(0, 1);
	de_init();
	de_layer_set(fb, fb1);
}
/*
void gr_dump(void)
{
	uart_printf("ccu tconlcd\n");
	uart_dump_reg(0x2001b60, 1);
	uart_printf("ccu video0\n");
	uart_dump_reg(0x2001040, 1);
	uart_printf("tcon dump\n");
	uart_dump_reg((uint32_t)TCON_LCD0, 0x100);
	uart_printf("ccu dump\n");
	uart_dump_reg(0x02001600, 4);
	uart_printf("blender dump\n");
	uart_dump_reg(0x05101000, 16*4);
	uart_printf("de ui1\n");
	uart_dump_reg(0x05103000, 16*4);
	uart_printf("de glb\n");
	uart_dump_reg(0x05100000, 16*4);
}
*/
void print_clocks(void)
{
	uart_printf("CPU clk: %d\n", ccu_cpu_clk_get());
	uart_printf("AHB clk: %d\n", ccu_ahb_clk_get());
	uart_printf("APB0 clk: %d\n", ccu_apb0_clk_get());
	uart_printf("APB1 clk: %d\n", ccu_apb1_clk_get());
	uart_printf("pll perip(x1): %d\n", ccu_perip_x1_clk_get());
}

uint32_t app_color[] = {
  C8888(255,   0,   0, 255),
  C8888(  0, 255,   0, 255),
  C8888(  0,   0, 255, 255),
  C8888(236, 245, 66, 255),
  C8888(66, 135, 245, 255),
  C8888(245, 191, 66, 255),
  C8888(245, 191, 242, 255),
  C8888(245, 191, 108, 255),
  C8888(66, 245, 78, 255),
};


void task_gr(void *arg)
{
	(void)arg;


	printf("gr\n");

	do_lcd();

	uint32_t d = 0;
	uint32_t t = 0;

	while(1)
	{
		uint32_t c = random_get() % ARRAY_SIZE(app_color);
		void *fb = de_layer_get_fb();

		if (t == 0) {
			uint32_t x0,x1,y0,y1;
			x0 = random_get() % (GR_X-1);
			x1 = random_get() % (GR_X-1);
			y0 = random_get() % (GR_Y-1);
			y1 = random_get() % (GR_Y-1);
			
			gr_draw_line(fb, x0, x1, y0, y1, app_color[c]);
		} else if (t == 1) {
			uint32_t x,y,r;
			x = random_get() % (GR_X-1);
			y = random_get() % (GR_Y-1);
			r = random_get() % (40);
			gr_draw_circle(fb, x, y, r, app_color[c]); 
		}
//		uart_printf("line %d %d %d %d\n", x0, y0, x1, y1);

		vTaskDelay(50);
		d++;
		if (d == 100) {
			gr_fill(fb, 0xff000000);
			d = 0;
			t++;
			if (t == 2) {
				t = 0;
			}
		}
	}

	vTaskDelete(NULL);
}

void task_init(void *arg)
{
	(void)arg;

	//syscall init
	syscalls_init();

	BaseType_t ret = xTaskCreate(task_blinky, "led", STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_blinky_handle);
	if (ret != pdTRUE){
		printf("not created\n");
		while(1);
	}
	
	ret = xTaskCreate(task_gr, "gr", 1000, NULL, tskIDLE_PRIORITY+2, &task_gr_handle);
	if (ret != pdTRUE){
		printf("not created\n");
		while(1);
	}

	usb_task_init();

	vTaskDelete(NULL);
}


int main()
{
//	*(uint32_t *)0x02500000 = '%';
	uart_printf("\n\nhello from allwinner\n");

	mmu_setup();
	mmu_enable();

	irq_init();
	dmac_init();

	ccu_init();
	led_init();

	memmgr_init();

	print_clocks();

	random_init(10);

	// now we switch to freertos
	BaseType_t ret = xTaskCreate(task_init, "init", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL);
	if (ret != pdTRUE){
		uart_printf("not created\n");
		while(1);
	}

	uart_printf("starting scheduler\n");
	vTaskStartScheduler();

	while(1);

	return 0;
}

void vApplicationMallocFailedHook(void)
{
	uart_printf("malloc fejld\n");

	while(1);

}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	(void)xTask;

	uart_printf("task stack overflow %s\n", pcTaskName);

	while(1);
}

void vApplicationIdleHook( void )
{
//	uart_printf("k\n");
}

#if configSUPPORT_STATIC_ALLOCATION
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
//uart_printf("idle get task memory %x %x %x\n", *ppxIdleTaskTCBBuffer, *ppxIdleTaskStackBuffer, *pulIdleTaskStackSize);
}

static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
//	uart_printf("timer get task memory %x %x %x\n", *ppxTimerTaskTCBBuffer, *ppxTimerTaskStackBuffer, *pulTimerTaskStackSize);
}
#endif
