#include "platform.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "led.h"
#include "dmac.h"
#include "irq.h"
#include "mmu.h"
#include "ccu.h"
#include "tcon_lcd.h"
#include "de.h"
#include "random.h"

#include "memmgr.h"
#include "event.h"

#include "ff.h"

#include "syscalls.h"
#include "uart.h"
#include "usb_task.h"
#include "doom_task.h"


#define STACK_SIZE 300
TaskHandle_t task_blinky_handle;

QueueHandle_t kbd_queue;

FATFS *fs;

void fs_init(void)
{
	DWORD fre_clust, fre_sect, tot_sect;

	fs = malloc(sizeof(FATFS));
	f_mount(fs, "", 0);

	if (f_getfree ("", &fre_clust, &fs) != FR_OK) {
		uart_printf("unable to mount disk\n");
		while(1);
	}

	tot_sect = (fs->n_fatent - 2) * fs->csize;
	fre_sect = fre_clust * fs->csize;
	uart_printf("%d KiB total drive space.\n%d KiB available.\n", tot_sect / 2, fre_sect / 2);
}


void task_blinky(void *arg)
{
	(void)arg;
	uint32_t state = 1;

	uart_printf("blinky\n");

	while(1)
	{
		led_set(0, state);
		state = !state;
		vTaskDelay(250);
	}
}

void print_clocks(void)
{
	uart_printf("CPU clk: %d\n", ccu_cpu_clk_get());
	uart_printf("AHB clk: %d\n", ccu_ahb_clk_get());
	uart_printf("APB0 clk: %d\n", ccu_apb0_clk_get());
	uart_printf("APB1 clk: %d\n", ccu_apb1_clk_get());
	uart_printf("pll perip(x1): %d\n", ccu_perip_x1_clk_get());
}

static void task_init(void *arg)
{
	(void)arg;

	syscalls_init();

	kbd_queue = xQueueCreate(10, sizeof(kbd_event_t));

	BaseType_t ret = xTaskCreate(task_blinky, "led", STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_blinky_handle);
	if (ret != pdTRUE){
		uart_printf("not created\n");
		while(1);
	}
	
	doom_task_init();
	usb_task_init();

	while(1);
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

	fs_init();

	BaseType_t ret = xTaskCreate(task_init, "init", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
	if (ret != pdTRUE){
		uart_printf("initial task not created\n");
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
