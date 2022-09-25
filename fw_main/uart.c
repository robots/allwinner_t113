//
// first uart is initialized by DRAM init code, it is 115200, uart_dma_mode == 0 (also after calling uart_init)
// 
// when uart_dma_mode == 0:
// - uart uses blocking access, one by one byte is transmitted when uart is not busy
//
// when uart_dma_mode == 1:
// - texts to print are stored in output fifo and transmission is done by DMA requests
// - access to send function is locked
//
// uart_init_dma needs to be called with FreeRTOS scheduller running (or between uart_init_dma
// and scheduler start, there can be no uart output) otherwise you get freeze.
//
#include "platform.h"

#include <stdarg.h>
#include <stdio.h>
#include "tinyprintf.h"

#include "gpio.h"
#include "ccu.h"
#include "dmac.h"
#include "fifo.h"
#include "mmu.h"

#include "uart.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define UART_DMAC_CH 0
#define UART_TX_BUF 4096

struct fifo_t uart_tx_fifo;

// both these are placed in non cached section
uint8_t __attribute__((section(".uart_ram"))) uart_tx_buf[UART_TX_BUF];
struct __attribute__((section(".uart_ram"), aligned(16))) dmac_desc_t uart_dmac_desc;

static uint32_t uart_sending_size;
static volatile int uart_sending;

static int uart_dma_mode = 0;
SemaphoreHandle_t uart_mutex;

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

static void uart_dmac_handler(uint8_t ch, void *arg);

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

void uart_init_dma(void)
{
	fifo_init(&uart_tx_fifo, uart_tx_buf, sizeof(uint8_t), UART_TX_BUF);

	uart_dmac_desc.config = BV(24) | (DMAC_DstReqUART0_TX << 16) | (DMAC_SrcReqDRAM); // DRAM source, UART0 TX dst, dst in IOmode, 8bit, no burst, 1byte block
	uart_dmac_desc.dst_addr = (void *)&UART0->DATA;
	uart_dmac_desc.parameter = 0;
	uart_dmac_desc.next = DMAC_NO_LINK;

	uart_mutex = xSemaphoreCreateMutex();
	if (uart_mutex == NULL) {
		uart_printf("unable to alloc mutex\n");
		while(1);
	}

	// wait for busy
	while ((UART0->UART_USR & 2) == 0);

	dmac_set_handler(UART_DMAC_CH, uart_dmac_handler, NULL);
	dmac_enable_irq(UART_DMAC_CH, BV(2)); // pkg end irq

	// enable TX dma
	UART0->UART_DMA_REQ_EN |= BV(1);

	uart_dma_mode = 1;
}

static void uart_dmac_handler(uint8_t ch, void *arg)
{
	(void)ch;
	(void)arg;

	fifo_read_done_size(&uart_tx_fifo, uart_sending_size);

	if (UNLIKELY(FIFO_EMPTY(&uart_tx_fifo))) {
		uart_sending = 0;
		uart_sending_size = 0;
		return;
	}

	uart_sending_size = fifo_get_read_size_cont(&uart_tx_fifo);

	uart_dmac_desc.src_addr = fifo_get_read_addr(&uart_tx_fifo);
	uart_dmac_desc.nbytes = uart_sending_size;

	// clear caches
	L1_CleanDCache_by_Addr(uart_dmac_desc.src_addr, uart_dmac_desc.nbytes);
	L1_CleanDCache_by_Addr(&uart_dmac_desc, sizeof(struct dmac_desc_t));

	dmac_start(UART_DMAC_CH, &uart_dmac_desc);
}

static void uart_send_dma(const char *buf, size_t count)
{
	xSemaphoreTake(uart_mutex, portMAX_DELAY);

#if 1
	// copy one by one, replace \n with \n\r
	while (1) {
		if (*buf == '\n') {
			fifo_write_more(&uart_tx_fifo, "\n\r", 2);
		} else {
			fifo_write_more(&uart_tx_fifo, buf, 1);
		}
		buf ++;
		count--;
		if (count == 0) {
			break;
		}

		if (fifo_get_free_space(&uart_tx_fifo) == 0) {
			break;
		}
	}
#else
	size_t wl = MIN(count, fifo_get_free_space(&uart_tx_fifo));
	fifo_write_more(&uart_tx_fifo, buf, wl);
#endif

	taskENTER_CRITICAL();

	if (UNLIKELY(uart_sending == 0)) {
		uart_sending = 1;

		uart_sending_size = fifo_get_read_size_cont(&uart_tx_fifo);

		uart_dmac_desc.src_addr = fifo_get_read_addr(&uart_tx_fifo);
		uart_dmac_desc.nbytes = uart_sending_size;

		// clear caches
		L1_CleanDCache_by_Addr(uart_dmac_desc.src_addr, uart_dmac_desc.nbytes);
		L1_CleanDCache_by_Addr(&uart_dmac_desc, sizeof(struct dmac_desc_t));

		dmac_start(UART_DMAC_CH, &uart_dmac_desc);
	}

	taskEXIT_CRITICAL();

	xSemaphoreGive(uart_mutex);
}

static void uart_putchar(char c)
{
	while ((UART0->UART_USR & 2) == 0);
	UART0->DATA = c;
	while ((UART0->UART_USR & 2) == 0);
}


static void uart_send_blocking(const char *buf, size_t count)
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

void uart_send(const char *buf, size_t count)
{
	if (uart_dma_mode == 0) {
		uart_send_blocking(buf, count);
	} else {
		uart_send_dma(buf, count);
	}
}

void uart_printf(const char *fmt, ...)
{
	char out[512];
	uint32_t len;

	va_list va;
	va_start(va, fmt);
	len = tfp_vsnprintf(out, 512, fmt, va);
	va_end(va);

	uart_send_blocking(out, len);
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
