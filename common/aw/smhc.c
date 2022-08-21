#include "platform.h"

#include <stdio.h>

#include "ccu.h"
#include "gpio.h"
#include "uart.h"

#include "smhc.h"

#define BLKSZ 0x200

struct gpio_t smhc_gpio[] = {
	{ // det
		.gpio = GPIOF,
		.pin = BV(6),
		.mode = GPIO_MODE_INPUT,
	},
	{ // sd pins
		.gpio = GPIOF,
		.pin = 0x7f, // PF0-PF5
		.mode = GPIO_MODE_FNC2,
		.drv = GPIO_DRV_3,
	},
};

enum smhc_card_t smhc_cardtype;

uint8_t smhc_databuf[BLKSZ] __attribute__ ((aligned(4)));

uint32_t smhc_cardrca;
uint32_t smhc_cardcid[4];
uint32_t smhc_cardcsd[4];
uint32_t smhc_cardsectors;
uint32_t smhc_cardsize;

struct smhc_cmd_t smhc_cmd;
struct smhc_dat_t smhc_dat;

void *memcpy(void *, const void*, size_t);

void dly(int loops)
{
	__asm__ __volatile__ (
		"1:\n"
		"subs %0, %1, #1\n"
		"bne 1b"
		:"=r" (loops):"0"(loops)
	);
}

static int smhc_clock_update(void)
{
	uint32_t timeout = 0xffffff;
	SMHC0->SMHC_CMD = BV(31) | BV(21) | BV(13);

	while (SMHC0->SMHC_CMD & BV(31)) {
		timeout --;
		if (timeout == 0) {
			printf("smhc: clk update timeout waiting\n");
			return SMHC_ERR;
		}
	}

	SMHC0->SMHC_RINTSTS = SMHC0->SMHC_RINTSTS;

	return SMHC_OK;
}

static int smhc_clock_set(uint32_t freq)
{
	int ret;
	uint32_t clk;

	SMHC0->SMHC_CLKDIV &= ~BV(16);

	CCU->SMHC0_CLK_REG |= BV(31);
	CCU->SMHC0_CLK_REG &= ~(3<<24);

	if (freq <= ccu_clk_hosc_get()) {
		CCU->SMHC0_CLK_REG |= 0; // hosc as source
		clk = ccu_clk_hosc_get();
	} else {
		CCU->SMHC0_CLK_REG |= 1 << 24; // peri 1x src (600mhz)
		clk = ccu_perip_x1_clk_get();
	}

	// calculate new div
	uint32_t div = (clk + freq * 2 - 1) / (2 * freq);
	if (div > 0xff) {
		printf("smhc: div too large\n");
		return SMHC_ERR;
	}

	printf("smhc: clk = %ld div %ld f_card = %ld\n", clk, div, clk/div/2);
	
	SMHC0->SMHC_CLKDIV = div;

	ret = smhc_clock_update();
	if (ret) return ret;

	SMHC0->SMHC_CLKDIV |= BV(17) | BV(16);

	ret = smhc_clock_update(); // needed ?
	if (ret) return ret;
/*
	printf("smhc: smhc_bgr_reg\n");
	dump_reg(0x200184c, 1);
	printf("smhc: smhc0 clk_reg\n");
	dump_reg(0x2001830, 1);
	printf("smhc: smhc0\n");
	dump_reg(0x04020000, 50);
*/
	return ret;
}

static void smhc_width_set(uint8_t d)
{
	if (d == 1) {
		SMHC0->SMHC_CTYPE = 0; // 1bit
	} else if (d == 4) {
		SMHC0->SMHC_CTYPE = 1; // 4bit
	} else {
		SMHC0->SMHC_CTYPE = 2; // 8bit
	}
}

static int smhc_rint_wait(uint32_t flag, uint32_t timeout)
{
	while ((SMHC0->SMHC_RINTSTS & flag) == 0) {
		timeout --;
		if (timeout == 0) {
			printf("smhc: rint timeout %08lx\n", flag);
			return SMHC_ERR;
		}
	}

	return SMHC_OK;
}

static int smhc_send_data(struct smhc_dat_t *dat)
{
	uint32_t flag;
	uint32_t count;
	uint32_t idx;

	if (dat->blk_sz & 3) {
		printf("smhc: blocksize not aligned to 4\n");
		while(1);
	}

	if (dat->flags & SMHC_DAT_READ) {
		flag = SMHC_STATUS_FIFO_EMPTY;
	} else {
		flag = SMHC_STATUS_FIFO_FULL;
	}

	// number of 32 bits to transfer
	count = dat->blks * dat->blk_sz >> 2;

	// access fifo from cpu
	SMHC0->SMHC_CTRL |= BV(31);

	idx = 0;
	while (count) {
		uint32_t timeout = 100000;
		while (SMHC0->SMHC_STATUS & flag) {
			timeout --;
			if (timeout == 0) {
				printf("smhc: data transfer timeout \n");
				return SMHC_ERR;
			}
		}

		if (dat->flags & SMHC_DAT_READ) {
			uint32_t tmp = SMHC0->SMHC_FIFO;
			memcpy(&dat->dat[idx], &tmp, sizeof(tmp));
		} else {
			uint32_t tmp;
			memcpy(&tmp, &dat->dat[idx], sizeof(tmp));
			SMHC0->SMHC_FIFO = tmp;
		}

		idx += 4;
		count -= 1;
	}

	return SMHC_OK;
}

static int smhc_send_cmd(struct smhc_cmd_t *cmd, struct smhc_dat_t *dat)
{
	uint32_t cmd_reg = SMHC_CMD_LOAD;
	int ret;

	if (cmd->cmd_idx == 12) {
		return SMHC_OK;
	}

	if (cmd->cmd_idx == 0) {
		cmd_reg |= SMHC_CMD_SEND_INIT_SEQ;
	}

	if (cmd->resp_type & SMHC_RESP_PRESENT) {
		cmd_reg |= SMHC_CMD_RESP_RCV;
	}
	if (cmd->resp_type & SMHC_RESP_136) {
		cmd_reg |= SMHC_CMD_LONG_RESP;
	}
	if (cmd->resp_type & SMHC_RESP_CRC) {
		cmd_reg |= SMHC_CMD_CHK_RESP_CRC;
	}

	if (dat) {
		cmd_reg |= SMHC_CMD_DATA_TRANS | SMHC_CMD_WAIT_PRE_OVER;

		if (dat->flags & SMHC_DAT_WRITE) {
			cmd_reg |= SMHC_CMD_TRANS_DIR;
		}
		if (dat->blks > 1) {
			cmd_reg |= SMHC_CMD_STOP_CMD_FLAG;
		}

		SMHC0->SMHC_BLKSIZ = dat->blk_sz;
		SMHC0->SMHC_BYTCNT = dat->blks * dat->blk_sz;
	}

	SMHC0->SMHC_CMDARG = cmd->cmd_arg;
	SMHC0->SMHC_CMD = cmd_reg | cmd->cmd_idx;

	if (dat) {
		ret = smhc_send_data(dat);
		if (ret) {
			printf("smhc: sennd dat err\n");
			return ret;
		}
	}

	ret = smhc_rint_wait(SMHC_RINT_CC, 1000000);
	if (ret == SMHC_ERR) {
		SMHC0->SMHC_CTRL = BV(0) | BV(1) | BV(2);
		smhc_clock_update();
		return SMHC_ERR;
	}

	if (dat) {
		uint32_t f;
		if (dat->blks > 1) {
			f = SMHC_RINT_ACD;
		} else {
			f = SMHC_RINT_DTC;
		}

		ret = smhc_rint_wait(f, 1000000);
		if (ret == SMHC_ERR) {
			SMHC0->SMHC_CTRL = BV(0) | BV(1) | BV(2);
			smhc_clock_update();
			return SMHC_ERR;
		}
	}

	if (cmd->resp_type & SMHC_RESP_BUSY) {
		uint32_t timeout = 500000;

		while(SMHC0->SMHC_STATUS & BV(9)) {
			timeout --;
			if (timeout == 0) {
				SMHC0->SMHC_CTRL = BV(0) | BV(1) | BV(2);
				smhc_clock_update();
				return SMHC_ERR;
			}
		}
	}

	cmd->resp[0] = SMHC0->SMHC_RESP0;
	cmd->resp[1] = SMHC0->SMHC_RESP1;
	if (cmd->resp_type & SMHC_RESP_136) {
		cmd->resp[2] = SMHC0->SMHC_RESP2;
		cmd->resp[3] = SMHC0->SMHC_RESP3;
	}
	
	SMHC0->SMHC_RINTSTS = SMHC0->SMHC_RINTSTS;
	SMHC0->SMHC_CTRL = BV(1);

	return SMHC_OK;
}

static int smhc_check_switchable(void)
{
	int ret;

	smhc_dat.blks = 1;
	smhc_dat.blk_sz = 64;
	smhc_dat.flags = SMHC_DAT_READ;
	smhc_dat.dat = smhc_databuf;

	smhc_cmd.cmd_idx = 6;
	smhc_cmd.cmd_arg = 0x00ffff01;
	smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC;
	ret = smhc_send_cmd(&smhc_cmd, &smhc_dat);

	if (ret) {
		printf("smhc:  cmd6 err\n");
		return ret;
	}

	if ((smhc_databuf[0] | smhc_databuf[1]) && ((smhc_databuf[28] | smhc_databuf[29]) == 0)) {

		// resend same command and check again
		ret = smhc_send_cmd(&smhc_cmd, &smhc_dat);
		if (ret) {
			printf("smhc:  cmd6 err #2\n");
			return ret;
		}

		if ((smhc_databuf[0] | smhc_databuf[1]) && ((smhc_databuf[28] | smhc_databuf[29]) == 0)) {
			return SMHC_OK; 
		}
	}

	return SMHC_ERR;
}

static int smhc_switch_bus_and_speed(int width4bit)
{
	int ret;

	// select card
	smhc_cmd.cmd_idx = 7;
	smhc_cmd.cmd_arg = smhc_cardrca;
	smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC;
	ret = smhc_send_cmd(&smhc_cmd, 0);

	if (ret == SMHC_ERR) {
		printf("smhc: error selecting card\n");
		return SMHC_ERR;
	}

	if (smhc_cardtype == SMHC_CARD_SDHC) {
		// acmd next
		smhc_cmd.cmd_idx = 55;
		smhc_cmd.cmd_arg = smhc_cardrca;
		smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC;
		ret = smhc_send_cmd(&smhc_cmd, 0);

		if (ret == SMHC_ERR) {
			printf("smhc: error 55\n");
			return ret;
		}

		smhc_dat.blks = 1;
		smhc_dat.blk_sz = 8;
		smhc_dat.dat = smhc_databuf; 
		smhc_dat.flags = SMHC_DAT_READ;

		// read SCR
		smhc_cmd.cmd_idx = 51;
		smhc_cmd.cmd_arg = 0;
		smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC;
		ret = smhc_send_cmd(&smhc_cmd, &smhc_dat);

		if (ret) {
			printf("smhc: error 51\n");
			return ret;
		}

		if ((smhc_databuf[0] & 0x0f) == 0x02) {
			if (smhc_check_switchable() == SMHC_OK) {
				printf("smhc: card supports switch, lets do 48MHz\n");
				ret = smhc_clock_set(48000000);
				if (ret) return ret;
			}
		}

	} else if (smhc_cardtype == SMHC_CARD_SDSC) {
		printf("smhc: card switching to 48MHz\n");
		ret = smhc_clock_set(48000000);
		if (ret) return ret;
	} else if (smhc_cardtype == SMHC_CARD_MMC) {
		// TODO: set max speed for mmc ... 
		width4bit = 0;
	}

	if (width4bit) {
		// set 4 bit bus
		printf("smhc: card switching to 4bit xfer\n");
		
		// acmd next
		smhc_cmd.cmd_idx = 55;
		smhc_cmd.cmd_arg = smhc_cardrca;
		smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC;
		ret = smhc_send_cmd(&smhc_cmd, 0);

		if (ret == SMHC_ERR) {
			printf("smhc: error 55 #2\n");
			return ret;
		}

		// select 4bit (ACMD6)
		smhc_cmd.cmd_idx = 6;
		smhc_cmd.cmd_arg = 2;
		smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC;
		ret = smhc_send_cmd(&smhc_cmd, 0);
		if (ret == SMHC_ERR) {
			printf("smhc: error acmd6\n");
			return ret;
		}

		smhc_width_set(4);
	}

	// set block size to 512
	smhc_cmd.cmd_idx = 16;
	smhc_cmd.cmd_arg = BLKSZ;
	smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC | SMHC_RESP_BUSY;
	ret = smhc_send_cmd(&smhc_cmd, 0);

	// deselect card
	smhc_cmd.cmd_idx = 7;
	smhc_cmd.cmd_arg = 0;
	smhc_cmd.resp_type = SMHC_RESP_NONE;
	ret = smhc_send_cmd(&smhc_cmd, 0);
	
	return ret;
}

int smhc_init(void)
{
	int ret;

	CCU->SMHC_BGR_REG |= BV(16);
	CCU->SMHC_BGR_REG |= BV(0);

	gpio_init(smhc_gpio, ARRAY_SIZE(smhc_gpio));

	smhc_width_set(1);
	smhc_clock_set(400000);

	// reset card
	smhc_cmd.cmd_idx = 0;
	smhc_cmd.cmd_arg = 0;
	smhc_cmd.resp_type = SMHC_RESP_NONE;
	smhc_send_cmd(&smhc_cmd, 0);
	dly(100);

	// voltage
	smhc_cmd.cmd_idx = 8;
	smhc_cmd.cmd_arg = 0x00000155;
	smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT;
	ret = smhc_send_cmd(&smhc_cmd, 0);

	// ident card
	if (ret == SMHC_OK) {
		printf("smhc: sd 2.0\n");

		do {
			// acmd next
			smhc_cmd.cmd_idx = 55;
			smhc_cmd.cmd_arg = 0;
			smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT;
			smhc_send_cmd(&smhc_cmd, 0);

			// host capacity support information comand
			smhc_cmd.cmd_idx = 41;
			smhc_cmd.cmd_arg = 0x40ff8000;
			smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT | SMHC_RESP_136;
			smhc_send_cmd(&smhc_cmd, 0);
		} while ((smhc_cmd.resp[3] & BV(31)) == 0);

		if (smhc_cmd.resp[3] & BV(30)) {
			// SDHC/SDXC
			smhc_cardtype = SMHC_CARD_SDHC;
			printf("smhc: sdhc\n");
		} else {
			// SDSC
			smhc_cardtype = SMHC_CARD_SDSC;
			printf("smhc: sdsc\n");
		}
	} else {
		smhc_cmd.cmd_idx = 0;
		smhc_cmd.cmd_arg = 0;
		smhc_cmd.resp_type = SMHC_RESP_NONE;
		smhc_send_cmd(&smhc_cmd, 0);

		dly(1000);

		smhc_cmd.cmd_idx = 55;
		smhc_cmd.cmd_arg = 0;
		smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT;
		ret = smhc_send_cmd(&smhc_cmd, 0);

		if (ret == SMHC_ERR) {
			printf("smhc: todo mmc card\n");
			return SMHC_ERR;
		} else {
			// host capacity support information comand
			smhc_cmd.cmd_idx = 41;
			smhc_cmd.cmd_arg = 0x40ff8000;
			smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT | SMHC_RESP_136;
			smhc_send_cmd(&smhc_cmd, 0);

			while ((smhc_cmd.resp[3] & 0x80000000) == 0) {
				// acmd next
				smhc_cmd.cmd_idx = 55;
				smhc_cmd.cmd_arg = 0;
				smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT;
				smhc_send_cmd(&smhc_cmd, 0);

				// host capacity support information comand
				smhc_cmd.cmd_idx = 41;
				smhc_cmd.cmd_arg = 0x40ff8000;
				smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT | SMHC_RESP_136;
				smhc_send_cmd(&smhc_cmd, 0);
			}

			smhc_cardtype = SMHC_CARD_SDSC;
		}
	}


	if (smhc_cardtype != SMHC_CARD_NONE) {
		// get cid
		smhc_cmd.cmd_idx = 2;
		smhc_cmd.cmd_arg = 0;
		smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT | SMHC_RESP_136;
		smhc_send_cmd(&smhc_cmd, 0);

		/*
		smhc_cardcid[0] = smhc_cmd.resp[3];
		smhc_cardcid[1] = smhc_cmd.resp[2];
		smhc_cardcid[2] = smhc_cmd.resp[1];
		smhc_cardcid[3] = smhc_cmd.resp[0];

		printf("cid = %08x\n", smhc_cardcid[0]);
		printf("cid = %08x\n", smhc_cardcid[1]);
		printf("cid = %08x\n", smhc_cardcid[2]);
		printf("cid = %08x\n", smhc_cardcid[3]);
		*/

		if (smhc_cardtype != SMHC_CARD_MMC) {
			smhc_cmd.cmd_idx = 3;
			smhc_cmd.cmd_arg = 0;
			smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT | SMHC_RESP_136;
			ret = smhc_send_cmd(&smhc_cmd, 0);
			if (ret == SMHC_ERR) {
				printf("smhc: rca err\n");
				return SMHC_ERR;
			}

			smhc_cardrca = smhc_cmd.resp[3];
		} else {
			printf("smhc: todocard mmc\n");
		}
		
		// get csd and calculate card size
		smhc_cmd.cmd_idx = 9;
		smhc_cmd.cmd_arg = smhc_cardrca;
		smhc_cmd.resp_type = SMHC_RESP_CRC | SMHC_RESP_PRESENT | SMHC_RESP_136;
		smhc_send_cmd(&smhc_cmd, 0);

		smhc_cardcsd[0] = smhc_cmd.resp[3];
		smhc_cardcsd[1] = smhc_cmd.resp[2];
		smhc_cardcsd[2] = smhc_cmd.resp[1];
		smhc_cardcsd[3] = smhc_cmd.resp[0];

		printf("smhc: csd = %08lx%08lx%08lx%08lx\n", smhc_cardcsd[0], smhc_cardcsd[1], smhc_cardcsd[2], smhc_cardcsd[3]);

		if (((smhc_cardcsd[0] &  0xc0000000) == 0) || (smhc_cardtype == SMHC_CARD_MMC)) {
			smhc_cardsectors = ((((smhc_cardcsd[1] & 0x03ff) << 2) | (smhc_cardcsd[2] >> 30)) + 1) << (((smhc_cardcsd[2] >> 15) & 0x07) + 2) << ((smhc_cardcsd[1] >> 16) & 0x0f) >> 9;
			smhc_cardsize = smhc_cardsectors >> 2;
		} else {
			smhc_cardsize = ((((smhc_cardcsd[1] & 0x03ff) << 16) | (smhc_cardcsd[2] >> 16)) + 1) * 512;
			smhc_cardsectors = smhc_cardsize * 2;
		}
		printf("smhc: sectors = %d\n", smhc_cardsectors);
		printf("smhc: size = %dkB\n", smhc_cardsize);

		smhc_switch_bus_and_speed(1);
	}
	
	return ret;
}

int smhc_read(uint32_t sect, uint32_t blocks, uint8_t *buf)
{
	int ret;

	// select card
	smhc_cmd.cmd_idx = 7;
	smhc_cmd.cmd_arg = smhc_cardrca;
	smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC | SMHC_RESP_BUSY;
	ret = smhc_send_cmd(&smhc_cmd, 0);

	if (ret == SMHC_OK) {
		// prepare transfer
		smhc_dat.blks = blocks;
		smhc_dat.blk_sz = 512;
		smhc_dat.flags = SMHC_DAT_READ;
		smhc_dat.dat = buf;

		if (blocks == 1) {
			smhc_cmd.cmd_idx = 17;
		} else {
			smhc_cmd.cmd_idx = 18;
		}

		if (smhc_cardtype != SMHC_CARD_SDHC) {
			smhc_cmd.cmd_arg = sect << 9;
		} else {
			smhc_cmd.cmd_arg = sect;
		}

		smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC | SMHC_RESP_BUSY;
		ret = smhc_send_cmd(&smhc_cmd, &smhc_dat);

		if (ret == SMHC_OK) {
			// deselect card
			smhc_cmd.cmd_idx = 7;
			smhc_cmd.cmd_arg = 0;
			smhc_cmd.resp_type = SMHC_RESP_NONE;
			ret = smhc_send_cmd(&smhc_cmd, 0);
		}
	
	}

	return ret;
}

int smhc_write(uint32_t sect, uint32_t blocks, uint8_t *buf)
{
	int ret;

	// select card
	smhc_cmd.cmd_idx = 7;
	smhc_cmd.cmd_arg = smhc_cardrca;
	smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC | SMHC_RESP_BUSY;
	ret = smhc_send_cmd(&smhc_cmd, 0);

	if (ret == SMHC_OK) {
		// prepare transfer
		smhc_dat.blks = blocks;
		smhc_dat.blk_sz = 512;
		smhc_dat.flags = SMHC_DAT_WRITE;
		smhc_dat.dat = buf;

		if (blocks == 1) {
			smhc_cmd.cmd_idx = 24;
		} else {
			smhc_cmd.cmd_idx = 25;
		}

		if (smhc_cardtype != SMHC_CARD_SDHC) {
			smhc_cmd.cmd_arg = sect << 9;
		} else {
			smhc_cmd.cmd_arg = sect;
		}

		smhc_cmd.resp_type = SMHC_RESP_PRESENT | SMHC_RESP_CRC | SMHC_RESP_BUSY;
		ret = smhc_send_cmd(&smhc_cmd, &smhc_dat);

		if (ret == SMHC_OK) {
			//deselect card
			smhc_cmd.cmd_idx = 7;
			smhc_cmd.cmd_arg = 0;
			smhc_cmd.resp_type = SMHC_RESP_NONE;
			ret = smhc_send_cmd(&smhc_cmd, 0);
		}
	
	}

	return ret;

}

size_t smhc_card_get_sectors(void)
{
	return smhc_cardsectors;
}

size_t smhc_card_get_block_size(void)
{
	return BLKSZ;
}
