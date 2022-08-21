// TODO external interrupt

#ifndef _GPIO_h_
#define _GPIO_h_

#include <stdint.h>

enum gpio_state_t {
	GPIO_RESET,
	GPIO_SET,
};

enum gpio_mode_t {
	GPIO_MODE_INPUT = 0,
	GPIO_MODE_OUTPUT,
	GPIO_MODE_FNC2,
	GPIO_MODE_FNC3,
	GPIO_MODE_FNC4,
	GPIO_MODE_FNC5,
	GPIO_MODE_FNC6,
	GPIO_MODE_FNC7,
	GPIO_MODE_FNC8,
	// Fnc 9-13 are reserved
	GPIO_MODE_EINT = 14,
	GPIO_MODE_DISABLED = 15,
};

enum gpio_pupd_t {
	GPIO_PUPD_OFF = 0,
	GPIO_PUPD_UP,
	GPIO_PUPD_DOWN,
};

enum gpio_drv_t {
	GPIO_DRV_0 = 0,
	GPIO_DRV_1,
	GPIO_DRV_2,
	GPIO_DRV_3,
};

struct gpio_t {
	GPIO_TypeDef *gpio;
	uint32_t pin;
	enum gpio_mode_t mode;
	enum gpio_pupd_t pupd; // pullup pulldown config
	enum gpio_drv_t drv; // drive strength
	enum gpio_state_t state; // initial state
};


void gpio_init(struct gpio_t *tab, size_t count);
void gpio_set(struct gpio_t *gpio, enum gpio_state_t state);
enum gpio_state_t gpio_get(struct gpio_t *gpio);

#endif
