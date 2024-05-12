#include "platform.h"

#include "ccu.h"

uint32_t ccu_clk_hosc24 = 24000000;
uint32_t ccu_clk_cpu = 408000000;
uint32_t ccu_clk_perip_x1 = 24000000;
uint32_t ccu_clk_video0_x4 = 24000000;
uint32_t ccu_clk_ahb = 24000000;
uint32_t ccu_clk_apb0 = 24000000;
uint32_t ccu_clk_apb1 = 24000000;

inline void dly(int loops) __attribute__ ((always_inline));
inline void dly(int loops)
{
	__asm__ __volatile__ (
		"1:\n"
		"subs %0, %1, #1\n"
		"bne 1b"
		:"=r" (loops):"0"(loops)
	);
}

void ccu_de_set_peripx2_div(uint32_t div)
{
	CCU->DE_CLK_REG = (0 << 24) | (0 << 8) | (div-1); // src = peri(x2), n=1, m=div
	CCU->DE_CLK_REG |= BV(31);
}

void ccu_de_enable(void)
{
	CCU->DE_BGR_REG |= BV(16);
	CCU->DE_BGR_REG |= BV(0);
}

void ccu_tcon_set_video0x4_div(uint32_t div)
{
	CCU->TCONLCD_CLK_REG = (1 << 24) | (0 << 8) | (div-1); // src = video0(x4), n=1, m=div
	CCU->TCONLCD_CLK_REG |= BV(31);
}

void ccu_dsi_enable(void)
{
	CCU->DSI_CLK_REG = (1 << 24) | (0 << 8) | (0);
	CCU->DSI_CLK_REG |= BV(31);
	CCU->DSI_BGR_REG |= BV(16);
	CCU->DSI_BGR_REG |= BV(0);
}

void ccu_lvds_enable(void)
{
	CCU->LVDS_BGR_REG |= BV(16);
}

void ccu_tcon_lcd_enable(void)
{
	CCU->TCONLCD_BGR_REG |= BV(16);
	CCU->TCONLCD_BGR_REG |= BV(0);
}

uint32_t ccu_tcon_get_clk(void)
{
	uint32_t clk;
	uint32_t src = ((CCU->TCONLCD_CLK_REG >> 24) & 7);
	switch (src) {
		case 0:
			clk = ccu_clk_video0_x4 / 4;
			break;
		case 1:
			clk = ccu_clk_video0_x4;
			break;
		default:
			clk = 1; // meh do more :)
	}

	uint32_t n = 1 << ((CCU->TCONLCD_CLK_REG >> 8) & 2);
	uint32_t m = 1 + (CCU->TCONLCD_CLK_REG & 0xf);
	//uint32_t en = CCU->TCONLCD_CLK_REG & BV(31);

//	uart_printf("en = %d clk = %d n = %d m = %d\n", !!en, clk, n, m );
	return clk / n / m;
}

void ccu_twi_enable(TWI_TypeDef *twi)
{
	if (twi == TWI2) {
		CCU->TWI_BGR_REG |= BV(2);
		CCU->TWI_BGR_REG |= BV(18);
	}
}

void ccu_uart_enable(UART_TypeDef *uart)
{
	if (uart == UART0) {
		// enable clock 
		CCU->UART_BGR_REG |= BV(0);

		// release from reset
		CCU->UART_BGR_REG |= BV(16);
	} else if (uart == UART1) {
		// enable clock 
		CCU->UART_BGR_REG |= BV(1);

		// release from reset
		CCU->UART_BGR_REG |= BV(17);
	}
}

static void ccu_dma_enable(void)
{
	// deassert reset
	CCU->DMA_BGR_REG |= BV(16);
	dly(20);
	// enable clock
	CCU->DMA_BGR_REG |= BV(0);
}

static void ccu_mbus_enable(void)
{
	// deassert reset
	CCU->MBUS_CLK_REG |= BV(30);

	// enable clock to all mbus peripherals
	CCU->MBUS_MAT_CLK_GATING_REG = 0x00000d87;
}

uint32_t ccu_cpu_to_clk(void)
{
	uint32_t n = 1 + ((CCU->PLL_CPU_CTRL_REG >> 8) & 0xff);
	uint32_t m = 1 + ((CCU->PLL_CPU_CTRL_REG >> 0) & 2);

	return ccu_clk_hosc24 * n / m;
}

static void ccu_cpu_pll_set(uint8_t n)
{
	uint32_t reg;

	// CPU clock source to osc24m Select cpux clock src to osc24m, axi divide ratio is 3, system apb clk ratio is 4 
	CCU->CPU_AXI_CFG_REG = (0 << 24) | (3 << 8) | (1 << 0);
	dly(1);

	// enable pll output
	CCU->PLL_CPU_CTRL_REG &= ~(1 << 27);

	// enable pll ldo
	CCU->PLL_CPU_CTRL_REG |= (1 << 30);
	dly(5);

	// set PLL CPU clock (24*42 = 1008) 
	reg = CCU->PLL_CPU_CTRL_REG;
	reg &= ~((0x3 << 16) | (0xff << 8) | (0x3 << 0));
	// (41+1) * 24MHz = 1008
	reg |= ((n-1) << 8);
	CCU->PLL_CPU_CTRL_REG = reg;

	// Lock enable
	CCU->PLL_CPU_CTRL_REG |= BV(29);

	// enable pll
	CCU->PLL_CPU_CTRL_REG |= BV(31);

	// wait for pll lock/
	while ((CCU->PLL_CPU_CTRL_REG & BV(28)) == 0);
	dly(20);

	// Enable pll gating
	CCU->PLL_CPU_CTRL_REG |= BV(27);

	// Lock disable - is this needed ? 
//	CCU->PLL_CPU_CTRL_REG &= ~(1 << 29);
//	dly(1);

	// change CPU clock source
	reg = CCU->CPU_AXI_CFG_REG;
	reg &= ~(0x07 << 24 | (0x3 << 16) | (0x3 << 8) | (0xf << 0));
	// src is PLLCPU/P, P=1, DIV1 = 4, DIV2 = 2
	reg |= ((0x03 << 24) | (0x0 << 16) | (0x3 << 8) | (0x1 << 0));
	CCU->CPU_AXI_CFG_REG = reg;
	dly(1);

	ccu_clk_cpu = ccu_cpu_to_clk();
}

uint32_t ccu_perip_x1_to_clk(void)
{
	uint32_t n = 1 + ((CCU->PLL_PERI_CTRL_REG >> 8) & 0xff);
	uint32_t m = 1 + ((CCU->PLL_PERI_CTRL_REG >> 1) & 1);
	uint32_t p0 = 1 + ((CCU->PLL_PERI_CTRL_REG >> 16) & 3);

	return ccu_clk_hosc24 * n / m / p0 / 2;
}

static void ccu_perip_pll_set(void)
{
	/* Periph0 has been enabled */
	if (CCU->PLL_PERI_CTRL_REG & (1 << 31)) {
		// already setup by ddr enable code
		ccu_clk_perip_x1 = ccu_perip_x1_to_clk();
		return;
	}

	// leave default value of 1.2GHz
	//CCU->PLL_PERI_CTRL_REG = 0x48216300;

	// Lock enable
	CCU->PLL_PERI_CTRL_REG |= (1 << 29);

	// enable pll
	CCU->PLL_PERI_CTRL_REG |= (1 << 31);

	// wait for pll lock/
	while (!(CCU->PLL_PERI_CTRL_REG & BV(28)));
	dly(20);

	// Lock disable - needed ?
	//CCU->PLL_PERI_CTRL_REG &= ~(1 << 29);
}

uint32_t ccu_video0_x4_to_clk(void)
{
	uint32_t n = 1 + ((CCU->PLL_VIDEO0_CTRL_REG >> 8) & 0xff);
	uint32_t m = 1 + ((CCU->PLL_VIDEO0_CTRL_REG >> 1) & 1);

//	uart_printf("video0: n=%d m=%d ccu_clk_host24 = %d\n", n, m, ccu_clk_hosc24);
	return ccu_clk_hosc24 * n / m;
}

void ccu_video0_pll_set(uint8_t n, uint8_t m)
{
	// default 1188MHz
	CCU->PLL_VIDEO0_CTRL_REG &= ~BV(31);

	if (m == 1) {
		CCU->PLL_VIDEO0_CTRL_REG &= ~BV(1);
	} else if (m == 2) { 
		CCU->PLL_VIDEO0_CTRL_REG |= BV(1);
	}
	
	CCU->PLL_VIDEO0_CTRL_REG &= ~(0xff << 8);
	CCU->PLL_VIDEO0_CTRL_REG |= (n-1) << 8;

	CCU->PLL_VIDEO0_CTRL_REG |= BV(31) | BV(30); // enable pll, ldo
	CCU->PLL_VIDEO0_CTRL_REG |= BV(29); // lock enable

	while (!(CCU->PLL_VIDEO0_CTRL_REG & BV(28))); // wait for pll stable
	dly(20);

	//CCU->PLL_VIDEO0_CTRL_REG &= ~BV(29); // lock disable

	ccu_clk_video0_x4 = ccu_video0_x4_to_clk();
}

void ccu_video1_pll_set(uint8_t n, uint8_t m)
{
	// default 1188MHz
	CCU->PLL_VIDEO1_CTRL_REG &= ~BV(31);

	if (m == 1) {
		CCU->PLL_VIDEO1_CTRL_REG &= ~BV(1);
	} else if (m == 2) { 
		CCU->PLL_VIDEO1_CTRL_REG |= BV(1);
	}
	
	
	CCU->PLL_VIDEO1_CTRL_REG &= ~(0xff << 8);
	CCU->PLL_VIDEO1_CTRL_REG |= (n-1) << 8;

	CCU->PLL_VIDEO1_CTRL_REG |= BV(31) | BV(30); // enable pll, ldo
	CCU->PLL_VIDEO1_CTRL_REG |= BV(29); // lock enable

	while (!(CCU->PLL_VIDEO1_CTRL_REG & BV(28))); // wait for pll stable
	dly(20);

	//CCU->PLL_VIDEO1_CTRL_REG &= ~BV(29); // lock disable

	//ccu_clk_video1_x4 = ccu_video1_x4_to_clk();
}

static void ccu_ve_pll_set(void)
{
	// default 432MHz
	if(!(CCU->PLL_VE_CTRL_REG & BV(31))) {
		CCU->PLL_VE_CTRL_REG |= BV(31) | BV(30); // enable pll, ldo
		CCU->PLL_VE_CTRL_REG |= BV(29); // lock enable

		while (!(CCU->PLL_VE_CTRL_REG & BV(28))); // wait for pll stable
		dly(20);

		//CCU->PLL_VE_CTRL_REG &= ~BV(29); // lock disable
	}
}

static void ccu_audio0_pll_set(void)
{
	// default AUDIO0(1x) 24.5714MHz
	if(!(CCU->PLL_AUDIO0_CTRL_REG & BV(31))) {
		CCU->PLL_AUDIO0_CTRL_REG |= BV(31) | BV(30); // enable pll, ldo
		CCU->PLL_AUDIO0_CTRL_REG |= BV(29); // lock enable

		while (!(CCU->PLL_AUDIO0_CTRL_REG & BV(28))); // wait for pll stable
		dly(20);

		//CCU->PLL_AUDIO0_CTRL_REG &= ~BV(29); // lock disable
	}
}

static void ccu_audio1_pll_set(void)
{
	// default AUDIO1 3072MHz
	if(!(CCU->PLL_AUDIO1_CTRL_REG & BV(31))) {
		CCU->PLL_AUDIO1_CTRL_REG |= BV(31) | BV(30); // enable pll, ldo
		CCU->PLL_AUDIO1_CTRL_REG |= BV(29); // lock enable

		while (!(CCU->PLL_AUDIO1_CTRL_REG & BV(28))); // wait for pll stable
		dly(20);

		//CCU->PLL_AUDIO1_CTRL_REG &= ~BV(29); // lock disable
	}
}

static void ccu_clk_ahb_set(int hosc)
{
	if (hosc) {
		// Change psi src to osc24m
		CCU->PSI_CLK_REG &= ~(0x3 << 24);

		ccu_clk_ahb = ccu_clk_hosc24;
	} else {
		//  600MHz / 3 / 1 = 200Mhz
		CCU->PSI_CLK_REG = (2 << 0) | (0 << 8);
		// peri(1x) pll as clk source 
		CCU->PSI_CLK_REG |= (0x03 << 24);

		ccu_clk_ahb = ccu_clk_perip_x1 / 3;
	}
}

static void ccu_clk_apb0_set(int hosc)
{
	if (hosc) {
		CCU->APB0_CLK_REG &= ~(0x03 << 24);

		ccu_clk_apb0 = ccu_clk_hosc24;
	} else {
		//  600MHz / 3 / 2 = 100MHz
		CCU->APB0_CLK_REG = (2 << 0) | (1 << 8);
		// peri(1x) pll as clk source
		CCU->APB0_CLK_REG |= 0x03 << 24;

		ccu_clk_apb0 = ccu_clk_perip_x1 / 3 / 2;
	} 
}

void ccu_init(void)
{
	ccu_cpu_pll_set(42);

	ccu_clk_ahb_set(1);
	ccu_clk_apb0_set(1);

	ccu_perip_pll_set();

	ccu_clk_ahb_set(0);
	ccu_clk_apb0_set(0);

	ccu_mbus_enable();
	ccu_dma_enable();

	//ccu_video0_pll_set();
	//ccu_video1_pll_set();
	//ccu_ve_pll_set();
	//ccu_audio0_pll_set();
	//ccu_audio1_pll_set();
}

uint32_t ccu_perip_x1_clk_get(void)
{
	return ccu_clk_perip_x1;
}

uint32_t ccu_video0_x4_clk_get(void)
{
	return ccu_clk_video0_x4;
}

uint32_t ccu_clk_hosc_get(void)
{
	return ccu_clk_hosc24;
}

uint32_t ccu_apb1_clk_get(void)
{
	return ccu_clk_apb1;
}

uint32_t ccu_apb0_clk_get(void)
{
	return ccu_clk_apb0;
}

uint32_t ccu_ahb_clk_get(void)
{
	return ccu_clk_ahb;
}

uint32_t ccu_cpu_clk_get(void)
{
	return ccu_clk_cpu;
}
