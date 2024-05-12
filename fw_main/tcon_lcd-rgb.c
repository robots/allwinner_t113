#include "platform.h"

#include "ccu.h"
#include "gpio.h"
#include "uart.h"
#include "irq.h"

#include "de.h"

#include "tcon_lcd.h"

#include "FreeRTOS.h"

struct timing_t {
	uint32_t pixclk;
	uint32_t w;
	uint32_t h;
	uint32_t hbp;
	uint32_t ht;
	uint32_t hspw;
	uint32_t vbp;
	uint32_t vt;
	uint32_t vspw;
} timing = {
	.pixclk = 48000000,
	.w = 800,
	.h = 480,
	.hbp = 65+1,
	.ht = 1055+1,
	.hspw = 20,
	.vbp = 33,
	.vt = 525,
	.vspw = 10,
};

struct gpio_t tcon_lcd_gpio[] = {
	{
		.gpio = GPIOD,
		.pin = 0x3fffff, // 0-21
		.mode = GPIO_MODE_FNC2, // lcd
		.drv = GPIO_DRV_3, // highest drv?
	},
};

void tcon_find_clock(uint32_t tgt_freq)
{
	uint32_t osc = ccu_clk_hosc_get();
	uint32_t best_n = 12;
	uint32_t best_m = 1;
	uint32_t best_d = 6;
	uint32_t best_err = 0xffffffff;

	uart_printf("tcon: looking up pll parameters for %dHz\n", tgt_freq);
	tgt_freq *=2;

	for (uint32_t n = 12; n < 100; n ++) {
		for (uint32_t m = 1; m < 3; m++) {
			for (uint32_t d = 6; d < 128; d ++) {
				uint32_t freq = osc * n / m / d;
				
				uint32_t err = ABS(freq - tgt_freq);
				if (err < best_err) {
					best_n = n;
					best_m = m;
					best_d = d;
					best_err = err;

					if (err == 0) {
						goto end;			
					}
				}
			}
		}
	}
end:
	
	uart_printf("tcon: best: n=%d m=%d d=%d err=%d\n", best_n, best_m, best_d, best_err);

	ccu_video0_pll_set(best_n, best_m);
	ccu_tcon_set_video0x4_div(1);
	ccu_tcon_lcd_enable();

	TCON_LCD0->DCLK_REG = best_d*2;
}

void tcon_dither(void)
{
	TCON_LCD0->FRM_SEED_REG[0] = 0x11111111;
	TCON_LCD0->FRM_SEED_REG[1] = 0x11111111;
	TCON_LCD0->FRM_SEED_REG[2] = 0x11111111;
	TCON_LCD0->FRM_SEED_REG[3] = 0x11111111;
	TCON_LCD0->FRM_SEED_REG[4] = 0x11111111;
	TCON_LCD0->FRM_SEED_REG[5] = 0x11111111;
	TCON_LCD0->FRM_TAB_REG[0] = 0x01010000;
	TCON_LCD0->FRM_TAB_REG[1] = 0x15151111;
	TCON_LCD0->FRM_TAB_REG[2] = 0x57575555;
	TCON_LCD0->FRM_TAB_REG[3] = 0x7f7f7777;
	TCON_LCD0->FRM_CTL_REG = BV(31);
}

static void tcon_int_handler(void *arg)
{
	(void)arg;
	uart_printf("tcon int\n");

	uint32_t gint0 = TCON_LCD0->GINT0_REG;

	if (gint0 & BV(15)) {
		TCON_LCD0->GINT0_REG = BV(15);

		de_int_vblank();
	}

	if (gint0 & BV(13)) {
		TCON_LCD0->GINT0_REG = BV(13);
	}
}

void tcon_lcd_init(void)
{
	uart_printf("tcon: init\n");
	gpio_init(tcon_lcd_gpio, ARRAY_SIZE(tcon_lcd_gpio));

	tcon_lcd_disable();

	tcon_find_clock(timing.pixclk);
	uart_printf("tcon_lcd: tcon clk = %dHz pixclk = %dHz\n", ccu_tcon_get_clk(), timing.pixclk);

	// init iface
	uint32_t val = timing.vt - timing.h - 8;
	if (val > 31) val = 31;
	if (val < 10) val = 10;
	TCON_LCD0->CTL_REG = ((val & 0x1f) << 4) |  0; // 7= grid test mode, 1=colorcheck, 2-grray chaeck

	TCON_LCD0->HV_IF_REG = 0; // 24bit/1cycle

	// init timing
	TCON_LCD0->BASIC0_REG = ((timing.w  - 1) << 16) | (timing.h - 1);
	TCON_LCD0->BASIC1_REG = ((timing.ht - 1) << 16) | (timing.hbp - 1);
	TCON_LCD0->BASIC2_REG = ((timing.vt * 2) << 16) | (timing.vbp - 1);
	TCON_LCD0->BASIC3_REG = ((timing.hspw)   << 16) | (timing.vspw);

	// io polarity for h,v,de,clk
	TCON_LCD0->IO_TRI_REG = 0; // default is 0xffffff (very bad :-)
	TCON_LCD0->IO_POL_REG = 2 << 28; // 2/3phase offset ?! why ?

	// enable line interrupt ...
	// install irq handler
	// TCON_LCD0->GINT1_REG = line << 16;
	// TCON_LCD0->GINT0_REG = BV(29);
	//
	irq_set_handler(TCON_LCD_IRQn, tcon_int_handler, NULL );
	irq_set_prio(TCON_LCD_IRQn, configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );
	irq_set_enable(TCON_LCD_IRQn, 1);

	//tcon_dither();
	uart_printf("tcon: init done\n");
}

void tcon_lcd_enable(void)
{
	TCON_LCD0->DCLK_REG |= (0x0f << 28);
	TCON_LCD0->CTL_REG |= BV(31);
	TCON_LCD0->GCTL_REG |= BV(31);
}

void tcon_lcd_disable(void)
{
	TCON_LCD0->DCLK_REG &= ~(0xf << 28);
	TCON_LCD0->CTL_REG = 0;
	TCON_LCD0->GCTL_REG &= ~BV(31);
}
