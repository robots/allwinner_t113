#include "platform.h"

#include "ccu.h"
#include "gpio.h"
#include "uart.h"
#include "irq.h"

#include "de.h"

#include "tcon_lcd.h"

#include "FreeRTOS.h"

#define FRAMERATE 60

typedef struct {
	volatile uint32_t res0[20];
	volatile uint32_t dphy_ana1;
	volatile uint32_t dphy_ana2;
	volatile uint32_t dphy_ana3;
	volatile uint32_t dphy_ana4;
	volatile uint32_t res1[4+4+24+4+8];
	volatile uint32_t combo_phy_reg0;
	volatile uint32_t combo_phy_reg1;
	volatile uint32_t combo_phy_reg2;
} DSICOMBO_t;

#define DSI0_BASE   0x05450000
#define DSI0        ((DSICOMBO_t *) (DSI0_BASE+0x1000))

struct timing_t {
	uint32_t pxclk;
	uint32_t w;
	uint32_t h;
	uint32_t hbp;
	uint32_t ht;
	uint32_t hspw;
	uint32_t vbp;
	uint32_t vt;
	uint32_t vspw;
} timing = {
	.pxclk = 51200000,
	.w = 1024,
	.h = 600,

	.ht = 1024+250+1,
	.hbp = 90+1,
	.hspw = 70,

	.vt = 600+35,
	.vbp = 13,
	.vspw = 10,
};

struct gpio_t tcon_lcd_gpio[] = {
	{ // LCD enable
		.gpio = GPIOE,
		.pin = BV(12),
		.mode = GPIO_MODE_OUTPUT,
		.drv = GPIO_DRV_3,
	},
	{
		.gpio = GPIOD,
		.pin = 0x3ff, // 0-9
		.mode = GPIO_MODE_FNC3, // lvds
		.drv = GPIO_DRV_3, // highest drv?
	},
};

void tcon_find_clock(uint32_t tgt_freq)
{
	uint32_t osc = ccu_clk_hosc_get();
	uint32_t best_n = 12;
	uint32_t best_m = 1;
//	uint32_t best_d = 6;
	uint32_t best_err = 0xffffffff;

//	uint32_t d = 1;
	uart_printf("tcon: looking up pll parameters for %dHz\n", tgt_freq);
	// TODO: why 2x ?
	tgt_freq *=2;

	for (uint32_t n = 12; n < 100; n ++) {
		for (uint32_t m = 1; m < 3; m++) {
			/*for (uint32_t d = 6; d < 128; d ++) */{
				uint32_t freq = osc * n / m;
				//uint32_t freq = osc * n / m / d;
				
				uint32_t err = ABS(freq - tgt_freq);
				if (err < best_err) {
					best_n = n;
					best_m = m;
					//best_d = d;
					best_err = err;

					if (err == 0) {
						goto end;			
					}
				}
			}
		}
	}
end:
	
	uart_printf("tcon: best: n=%d m=%d err=%d\n", best_n, best_m, best_err);

	ccu_video0_pll_set(best_n, best_m);
	ccu_tcon_set_video0x4_div(1);
	ccu_tcon_lcd_enable();
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

static void enable_combphy_lvds(void)
{
//	uart_printf("dsi phy before\n");
//	uart_dump_reg(DSI0, 72);

	DSI0->combo_phy_reg1 = 0x43;
	DSI0->combo_phy_reg0 = 0x1;
	vTaskDelay(1);
	DSI0->combo_phy_reg0 = 0x5;
	vTaskDelay(1);
	DSI0->combo_phy_reg0 = 0x7;
	vTaskDelay(1);
	DSI0->combo_phy_reg0 = 0xf;


	//DSI0->dphy_ana4 = 0x84000000;
	//DSI0->dphy_ana3 = 0x01040000;
	//DSI0->dphy_ana2 &= (0 << 1);
	//DSI0->dphy_ana1 = 0;


//	uart_printf("dsi phy\n");
//	uart_dump_reg(DSI0, 72);
}

static void disable_combphy_lvds(void)
{
	DSI0->combo_phy_reg1 = 0;
	DSI0->combo_phy_reg0 = 0;
	DSI0->dphy_ana4 = 0;
	DSI0->dphy_ana3 = 0;
	DSI0->dphy_ana1 = 0;
}


#define LVDS_EN BV(31)
#define LVDS_MODE_JEIDA BV(27)
#define LVDS_18BIT      BV(26)
#define LVDS_CLK_SEL    BV(20)
static void setup_lvds(void)
{
	TCON_LCD0->LVDS_IF_REG = LVDS_18BIT | LVDS_MODE_JEIDA | LVDS_CLK_SEL;
	TCON_LCD0->LVDS_IF_REG |= LVDS_EN;
	TCON_LCD0->LVDS1_IF_REG = TCON_LCD0->LVDS_IF_REG;
}

#define LVDS_ANA_C(x) (x << 13)
#define LVDS_ANA_V(x) (x << 22)
#define LVDS_ANA_PD(x) (x << 26)
#define LVDS_ANA_EN_LDO(x) (x << 1)
#define LVDS_ANA_EN_MB(x) ( x << 0)
#define LVDS_ANA_EN_DRVC(x) (x << 7)
#define LVDS_ANA_EN_DRVD(x) (x << 8)
#define LVDS_ANA_EN_24M(x) (x << 3)
#define LVDS_ANA_EN_LVDS(x) (x << 2)

static void enable_lvds(void)
{
	TCON_LCD0->LVDS_IF_REG |= BV(31);

#if 1
	enable_combphy_lvds();
#else
	TCON_LCD0->LVDS_ANA_REG[0] = 
		LVDS_ANA_C(2) |
		LVDS_ANA_V(3) |
		LVDS_ANA_PD(2);

	vTaskDelay(1);

	TCON_LCD0->LVDS_ANA_REG[0] |=
		LVDS_ANA_EN_24M(1) |
		LVDS_ANA_EN_LVDS(1) |
		LVDS_ANA_EN_MB(1);

	vTaskDelay(1);

	TCON_LCD0->LVDS_ANA_REG[0] |=
		LVDS_ANA_EN_DRVC(1) |
		LVDS_ANA_EN_DRVD(0x07); // 18bit colors
#endif
}

static void disable_lvds(void)
{
	TCON_LCD0->LVDS_IF_REG &= ~BV(31);
	TCON_LCD0->LVDS_ANA_REG[0] = 0;

	disable_combphy_lvds();
}

void tcon_lcd_init(void)
{
	uart_printf("tcon: init\n");
	gpio_init(tcon_lcd_gpio, ARRAY_SIZE(tcon_lcd_gpio));

	// enable lcd power
	gpio_set(&tcon_lcd_gpio[0], GPIO_SET);

	tcon_lcd_disable();

	uint32_t tcon_div = 7;

	tcon_find_clock(timing.pxclk * tcon_div);

  // lvds dclk / 7
	TCON_LCD0->DCLK_REG = tcon_div;
	TCON_LCD0->DCLK_REG |= (0x0f << 28);

	// TODO: where does this 2 come from ?
	uart_printf("tcon_lcd: tcon clk = %dHz pixclk = %dHz\n", ccu_tcon_get_clk() / tcon_div / 2, timing.pxclk);
	ccu_dsi_enable();
	ccu_lvds_enable();

	// init iface
	uint32_t val = timing.vt - timing.h - 8;
	if (val > 31) val = 31;
	if (val < 10) val = 10;
	TCON_LCD0->CTL_REG = ((val & 0x1f) << 4) |  0; // 7= grid test mode, 1=colorcheck, 2-grray chaeck

	TCON_LCD0->HV_IF_REG = 0; // 24bit/1cycle

	setup_lvds();

	// init timing
	TCON_LCD0->BASIC0_REG = ((timing.w  - 1) << 16) | (timing.h - 1);
	TCON_LCD0->BASIC1_REG = ((timing.ht - 1) << 16) | (timing.hbp - 1);
	TCON_LCD0->BASIC2_REG = ((timing.vt * 2) << 16) | (timing.vbp - 1);
	TCON_LCD0->BASIC3_REG = ((timing.hspw)   << 16) | (timing.vspw);

	// io polarity for h,v,de,clk
	TCON_LCD0->IO_TRI_REG = 0; // default is 0xffffff (very bad :-)
	TCON_LCD0->IO_POL_REG = 0;//2 << 28; // 2/3phase offset ?! why ?

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
	TCON_LCD0->CTL_REG |= BV(31);
	TCON_LCD0->GCTL_REG |= BV(31);

	enable_lvds();
}

void tcon_lcd_disable(void)
{
	TCON_LCD0->CTL_REG = 0;
	TCON_LCD0->GCTL_REG &= ~BV(31);

	disable_lvds();
}
