
#include "platform.h"

#include "memmgr.h"
#include "de.h"
#include "tcon_lcd.h"
#include "gr.h"


#include "FreeRTOS.h"
#include "task.h"

#include "doom_task.h"

static void gr_dump(void)
{
	uart_printf("ccu tconlcd\n");
	uart_dump_reg(0x2001b60, 1);
	uart_printf("ccu video0\n");
	uart_dump_reg(0x2001040, 1);
	uart_printf("tcon dump\n");
	uart_dump_reg((uint32_t )TCON_LCD0, 0x100);
	uart_printf("ccu dump\n");
	uart_dump_reg(0x02001600, 4);
	uart_printf("blender dump\n");
	uart_dump_reg(0x05101000, 16*4);
	uart_printf("de ui1\n");
	uart_dump_reg(0x05103000, 16*4);
	uart_printf("de glb\n");
	uart_dump_reg(0x05100000, 16*4);
}

void task_doom(void *arg)
{
	(void)arg;
	
	printf("doom task\n");

	//printf("pll video0(x4): %d\n", ccu_video0_x4_clk_get());

	void *fb = dma_memalign(128, 800*480*4);
	void *fb1 = dma_memalign(128, 800*480*4);

	uart_printf("fb addr: %08x and %08x\n", fb, fb1);

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

extern void D_DoomMain (void);
	D_DoomMain ();

	vTaskDelete(NULL);
}

void doom_task_init(void)
{

	BaseType_t ret = xTaskCreate(task_doom, "doom", 1000, NULL, tskIDLE_PRIORITY+2, NULL);
	if (ret != pdTRUE){
		printf("not created\n");
		while(1);
	}

}

