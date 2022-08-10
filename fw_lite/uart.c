#include "platform.h"

#include <stdarg.h>
#include "tinyprintf.h"

#include "gpio.h"
#include "ccu.h"

#include "uart.h"

struct gpio_t uart_gpio[] = {
	{
		.gpio = GPIOE,
		.pin = BV(2),
		.mode = GPIO_MODE_FNC6,
		.drv = GPIO_DRV_0,
	},
	{
		.gpio = GPIOE,
		.pin = BV(3),
		.mode = GPIO_MODE_FNC6,
		.drv = GPIO_DRV_0,
	},
};

void uart_init(uint32_t baudrate)
{
	gpio_init(uart_gpio, ARRAY_SIZE(uart_gpio));

	ccu_uart_enable(UART0);

	// calculare baudrate divisor
	uint32_t clk = ccu_apb1_clk_get();

	uint32_t br = clk/16/baudrate;

	UART0->DLH_IER = 0x0;
	UART0->IIR_FCR = 0xf7;
	UART0->UART_MCR = 0x0;

	// enable access to DLL DLH
	UART0->UART_LCR |= (1 << 7);

	// configure baudrate prescaler
	UART0->DATA = br & 0xff;
	UART0->DLH_IER = (br >> 8) & 0xff;

	// disable access to DLL DLH
	UART0->UART_LCR &= ~(1 << 7);

	// configure line config 8,n,1
	UART0->UART_LCR &= ~0x1f;
	UART0->UART_LCR |= (0x3 << 0) | (0 << 2) | (0x0 << 3);
}

void uart_send(const char *buf, size_t count)
{
	while(count)
	{
		uart_putchar(*buf);

		if (*buf == '\n') {
			uart_putchar('\r');
		}

		buf++;
		count--;
	}
}

void uart_putchar(char c)
{
	while ((UART0->UART_USR & 2) == 0);

	UART0->DATA = c;
	while ((UART0->UART_USR & 2) == 0);
}

void uart_printf(const char *fmt, ...)
{
	char out[512];
	uint32_t len;

	va_list va;
	va_start(va, fmt);
	len = tfp_vsnprintf(out, 512, fmt, va);
	va_end(va);

	uart_send(out, len);
}

void uart_dump_reg(uint32_t base, size_t count)
{
	uint32_t cnt = 0;

	while (count) {
		if (cnt == 0) {
			uart_printf("\n\r%08x: ", base);
		}
		uint32_t val = *(uint32_t *)base;
		uart_printf("%08x ", val);
		base += 4;
		cnt += 1;
		count --;
		if (cnt == 4) cnt = 0;

	}
	uart_printf("\n\r");
}
