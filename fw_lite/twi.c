#include "platform.h"

#include "gpio.h"
#include "ccu.h"

#include "uart.h"

#include "twi.h"

struct gpio_t twi_gpio[] = {
	{ // sck
		.gpio = GPIOE,
		.pin = BV(12),
		.mode = GPIO_MODE_FNC2,
		.drv = GPIO_DRV_3,
	},
	{ // sda
		.gpio = GPIOE,
		.pin = BV(13),
		.mode = GPIO_MODE_FNC2,
		.drv = GPIO_DRV_3,
	},
};

void twi_set_clock(TWI_TypeDef *twi, uint32_t tgtfreq)
{
	uint32_t freq;
	uint32_t delta;
	uint32_t apb1 = ccu_clk_apb1();

	uint32_t best = 0xffffffff, best_n = 7, best_m = 15;

	for (n = 0; n <= 7; n++) {
		for (m = 0; m <= 15; m++) {
			freq = apb1 / (10 * (m+1) * (1 << n));
			delta = tgtfreq - freq;
			if (delta < best) {
				best = delta;
				best_n = n;
				best_m = m;
			}
			if (delta == 0) {
				break;
			}
		}
	}

	uart_printf("twi: clk %d n=%d m=%d\n\r", best_freq, best_n, best_m);

	twi->CCR = (best_m << 3) | best_n;
}

void twi_init(void)
{
	gpio_init(twi_gpio, ARRAY_SIZE(twi_gpio));

	ccu_twi_enable(TWI2);

	uart_printf("twi: init\n\r");

	twi_set_clock(TWI2, 100000);
}
