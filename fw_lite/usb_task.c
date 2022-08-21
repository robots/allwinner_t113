#include "platform.h"

#include "irq.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tusb.h"
#include "portable/ehci/ehci_api.h"

TaskHandle_t task_usb_handle;



#define EHCI0_BASE (0x04101000)

#define EHCI1_BASE (0x04200000)
#define OHCI_BASE (0x04200000+0x400)

static void usb_hw_init(void);

static void task_usb(void *arg)
{
	(void)arg;

	printf("task: usb\n");

	usb_hw_init();

	tuh_init(0);

	while(1)
	{
		tuh_task();
	}
}

static void usb_int_handler(void *arg)
{
	(void)arg;

	tuh_int_handler(0);
}

void usb_task_init(void)
{

	BaseType_t ret = xTaskCreate(task_usb, "usb", 1000, NULL, tskIDLE_PRIORITY+2, &task_usb_handle);
	if (ret != pdTRUE){
		printf("not created\n");
		while(1);
	}
}

/*
#define SUNXI_USB_OFFSET_PHY_CTRL   0x10
#define SUNXI_USB_OFFSET_PHY_STATUS   0x24

#define SUNXI_USB_PHY_EFUSE_ADDR    0x03006218
#define SUNXI_USB_PHY_EFUSE_ADJUST    0x10000   //bit16
#define SUNXI_USB_PHY_EFUSE_MODE    0x20000   //bit17
#define SUNXI_USB_PHY_EFUSE_RES     0x3C0000  //bit18-21
#define SUNXI_USB_PHY_EFUSE_COM     0x1C00000 //bit22-24
#define SUNXI_USB_PHY_EFUSE_USB0TX    0x1C00000 //bit22-24
#define SUNXI_USB_PHY_EFUSE_USB1TX    0xE000000 //bit25-27
																								//
void phy_write(uint32_t base, uint8_t addr, uint8_t data, uint8_t len)
{
	volatile uint8_t *reg_ctl = (uint8_t *)(base + 810);
	volatile uint8_t *reg_addr = (uint8_t *)(base + 811);
	//volatile uint8_t *reg_sta = (uint8_t *)(base + 824);

	for (int j = 0; j < len; j++) {
		*reg_ctl |= BV(1);

		*reg_addr = addr+j;

		uint8_t r = *reg_ctl;
		r &= ~BV(7);
		r |= (data & 1) << 7;
		*reg_ctl |= r;

		*reg_ctl |= BV(0);
		*reg_ctl &= ~BV(0);


		*reg_ctl &= ~BV(1);
		data >>= 1;
	}
}

uint8_t phy_read(uint32_t base, uint8_t addr, uint8_t len)
{
	uint8_t ret = 0;
	volatile uint8_t *reg_ctl = (uint8_t *)(base + 810);
	volatile uint8_t *reg_addr = (uint8_t *)(base + 811);
	volatile uint8_t *reg_sta = (uint8_t *)(base + 824);

	*reg_ctl |= BV(1);

	for (int j = len; j > 0; j--) {
		*reg_addr = (addr + j - 1);
		for (volatile int i = 0; i < 4; i ++);

		ret <<= 1;
		ret |= (*reg_sta) & 1;
	}

	*reg_ctl &= ~BV(1);

	return ret;
}

void phy_print(uint32_t base)
{
	printf("phybase = %p\n", base);
	printf("phy %02x = %x\n", 0x1c, phy_read(base, 0x1c, 0x03));
	printf("phy %02x = %x\n", 0x30, phy_read(base, 0x30, 0x0D));
	printf("phy %02x = %x\n", 0x60, phy_read(base, 0x60, 0x0E));
	printf("phy %02x = %x\n", 0x40, phy_read(base, 0x40, 0x08));
}
*/

static void usb_hw_init(void)
{
	volatile uint32_t *usb_ctrl = (uint32_t * ) (EHCI1_BASE + 0x800);
	volatile uint32_t *phy_ctrl = (uint32_t * ) (EHCI1_BASE + 0x810);
	volatile uint32_t *portsc  = (uint32_t * ) (EHCI1_BASE + 0x054);


	CCU->USB1_CLK_REG |= 0x01 << 24; // usb clock from 24MHz
	CCU->USB1_CLK_REG |= BV(30) | BV(31); // rst USB1 phy, gating

	CCU->USB_BGR_REG |= BV(21); // rst EHCI1
	CCU->USB_BGR_REG |= BV(5); // gating EHCI1
	CCU->USB_BGR_REG |= BV(17); // rst oHCI1
	CCU->USB_BGR_REG |= BV(1); // gating oHCI1

	*phy_ctrl &= ~BV(3);
	*usb_ctrl |= BV(10) | BV(9) | BV(8) | BV(0);

	printf("phy_ctl = %08lx\n", *portsc);
	*portsc |= BV(13);

	printf("usb_ctl = %08lx\n", *portsc);

/*
	irq_set_handler(USB1_EHCI_IRQn, usb_int_handler, NULL);
	irq_set_prio(USB1_EHCI_IRQn, configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );
*/
	irq_set_handler(USB1_OHCI_IRQn, usb_int_handler, NULL);
	irq_set_prio(USB1_OHCI_IRQn, configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );

}

void hcd_int_enable(uint8_t rhport)
{
	(void)rhport;
	irq_set_enable(USB1_OHCI_IRQn, 1);
}

/*
bool hcd_init(uint8_t rhport)
{
  return ehci_init(rhport, (uint32_t) EHCI1_BASE, (uint32_t) EHCI1_BASE+0x10);
}

void hcd_int_enable(uint8_t rhport)
{
	(void)rhport;
	irq_set_enable(USB1_EHCI_IRQn, 1);
}

void hcd_int_disable(uint8_t rhport)
{
	(void)rhport;
	irq_set_enable(USB1_EHCI_IRQn, 0);
}
*/
