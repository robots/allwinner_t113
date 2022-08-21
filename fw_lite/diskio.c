/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* by grqd_xp                                                            */
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
#include "platform.h"

#include <time.h>

#include "ff.h"
#include "diskio.h"
#include "smhc.h"
#include "ffconf.h"


DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	(void)pdrv;

	int ret = smhc_init();

	if (ret == SMHC_OK) {
		return RES_OK;
	}	else {
		return RES_ERROR;
	}
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	(void)pdrv;
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	(void)pdrv;

	if (!count) {    
		return RES_PARERR;  // only supports single disk operation, count is not equal to 0, otherwise parameter error
	}

	int ret = smhc_read(sector, count, buff);

	// Process the return value, the return value of the SPI_SD_driver.c the return value turned into ff.c
	if (ret == SMHC_OK) {
		return RES_OK;
	}	else {
		return RES_ERROR;
	}
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	(void)pdrv;

	if (!count) {    
		return RES_PARERR;  // only supports single disk operation, count is not equal to 0, otherwise parameter error
	}

	int ret = smhc_write(sector, count, (uint8_t*)buff);

	// Return value to
	if (ret == SMHC_OK) {
		return RES_OK;
	} else {
		return RES_ERROR;
	}
}

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	(void)pdrv;
	DRESULT res;

	// FATFS only deal with the current version of CTRL_SYNC, GET_SECTOR_COUNT, GET_BLOCK_SIZ three commands
	switch(cmd) {
		case CTRL_SYNC:
//			if(SD_WaitReady()==0) {
				res = RES_OK;
/*			} else {
				res = RES_ERROR;
			}*/
			break;
		case GET_BLOCK_SIZE:
			*(WORD*)buff = smhc_card_get_block_size();
			res = RES_OK;
			break;

		case GET_SECTOR_COUNT:
			*(DWORD*)buff = smhc_card_get_sectors();
			res = RES_OK;
			break;
		default:
			res = RES_PARERR;
			break;
	
	}

	return res;
}

DWORD get_fattime(void)
{
	DWORD tmr = 0;

	time_t time = 0;
	struct tm * timeinfo;
	timeinfo = localtime(&time);

	tmr = (((DWORD)(timeinfo->tm_year) + 1900 - 1980) << 25)
		 | ((DWORD)(timeinfo->tm_mon+1) << 21)
		 | ((DWORD)(timeinfo->tm_mday) << 16)
		 | ((WORD)(timeinfo->tm_hour) << 11)
		 | ((WORD)(timeinfo->tm_min) << 5)
		 | ((WORD)(timeinfo->tm_sec) << 1);

	return tmr;
}

