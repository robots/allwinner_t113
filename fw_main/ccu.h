#ifndef _CCU_h_
#define _CCU_h_

#include "platform.h"

void ccu_init(void);

void ccu_de_set_peripx2_div(uint32_t div);
void ccu_de_enable(void);

void ccu_dsi_enable(void);
void ccu_lvds_enable(void);

void ccu_tcon_set_video0x4_div(uint32_t div);
void ccu_tcon_lcd_enable(void);
uint32_t ccu_tcon_get_clk(void);

void ccu_uart_enable(UART_TypeDef *uart);
void ccu_twi_enable(TWI_TypeDef *twi);

void ccu_video0_pll_set(uint8_t n, uint8_t m);
void ccu_video1_pll_set(uint8_t n, uint8_t m);

uint32_t ccu_clk_hosc_get(void);
uint32_t ccu_perip_x1_clk_get(void);
uint32_t ccu_video0_x4_clk_get(void);
uint32_t ccu_apb1_clk_get(void);
uint32_t ccu_apb0_clk_get(void);
uint32_t ccu_ahb_clk_get(void);
uint32_t ccu_cpu_clk_get(void);

#endif
