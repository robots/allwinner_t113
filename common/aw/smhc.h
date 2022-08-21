#ifndef SMHC_h_
#define SMHC_h_

#include "platform.h"

#define SMHC_CMD_LOAD           BV(31)

#define SMHC_CMD_SEND_INIT_SEQ  BV(15)

#define SMHC_CMD_WAIT_PRE_OVER  BV(13)
#define SMHC_CMD_STOP_CMD_FLAG  BV(12)
#define SMHC_CMD_TRANS_MODE     BV(11)
#define SMHC_CMD_TRANS_DIR      BV(10)
#define SMHC_CMD_DATA_TRANS     BV(9)
#define SMHC_CMD_CHK_RESP_CRC   BV(8)
#define SMHC_CMD_LONG_RESP      BV(7)
#define SMHC_CMD_RESP_RCV       BV(6)

#define SMHC_RINT_ACD           BV(14)
#define SMHC_RINT_DTC           BV(3)
#define SMHC_RINT_CC            BV(2)

#define SMHC_STATUS_FIFO_FULL   BV(3)
#define SMHC_STATUS_FIFO_EMPTY  BV(2)

#define SMHC_RESP_NONE    0
#define SMHC_RESP_PRESENT 1
#define SMHC_RESP_136     2
#define SMHC_RESP_CRC     4
#define SMHC_RESP_BUSY    8

#define SMHC_DAT_WRITE   1
#define SMHC_DAT_READ    2

enum {
	SMHC_OK = 0,
	SMHC_ERR,
};

enum smhc_card_t {
	SMHC_CARD_NONE = 0,
	SMHC_CARD_SDSC,
	SMHC_CARD_MMC,
	SMHC_CARD_SDHC,
};


struct smhc_cmd_t {
	uint16_t cmd_idx;
	uint32_t cmd_arg;
	uint32_t resp_type;
	uint32_t resp[4];
};

struct smhc_dat_t {
	uint8_t *dat;
	uint32_t flags;
	uint32_t blks;
	uint32_t blk_sz;
};

int smhc_init(void);
int smhc_read(uint32_t sect, uint32_t blocks, uint8_t *buf);
int smhc_write(uint32_t sect, uint32_t blocks, uint8_t *buf);
size_t smhc_card_get_sectors(void);
size_t smhc_card_get_block_size(void);

#endif
