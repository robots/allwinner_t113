#include "platform.h"

#include "gpio.h"

void gpio_init(struct gpio_t *tab, size_t count)
{
	for (size_t idx = 0; idx < count ; idx++) {

		gpio_set(&tab[idx], tab[idx].state);

		for (uint32_t pos = 0; pos < 32; pos++) {
			if ((tab[idx].pin & BV(pos)) == 0) {
				continue;
			}

			uint32_t cfg_idx = pos >> 3;
			uint32_t drv_idx = pos >> 3;
			uint32_t pull_idx = pos >> 2;

			GPIO_TypeDef *gpio = tab[idx].gpio;
			uint32_t reg;

			uint32_t shift = (pos & 7) * 4;
			reg = gpio->CFG[cfg_idx];
			reg &= ~(0x0f << shift);
			reg |= tab[idx].mode << shift;
			gpio->CFG[cfg_idx] = reg;;

			reg = gpio->DRV[drv_idx];
			reg &= ~(0x03 << shift);
			reg |= tab[idx].drv << shift;
			gpio->DRV[drv_idx] = reg;

			shift = (pos & 3) * 2;
			reg = gpio->PULL[pull_idx];
			reg &= ~(0x03 << shift);
			reg |= tab[idx].pupd << shift;
			gpio->PULL[pull_idx] = reg;

		}
	}
}

void gpio_set(struct gpio_t *gpio, enum gpio_state_t state)
{
	if (state == GPIO_SET) {
		gpio->gpio->DATA |= gpio->pin;
	} else {
		gpio->gpio->DATA &= ~gpio->pin;
	}
}

enum gpio_state_t gpio_get(struct gpio_t *gpio)
{
	return (gpio->gpio->DATA & gpio->pin) ? GPIO_SET : GPIO_RESET;
}
