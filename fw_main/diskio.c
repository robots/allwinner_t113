//
// "ramdisk" implementation using FAT16 precompiled image.
// The image is stored in external symbol "diskimg" with length in "diskimg_length"
// this image file is created using bin2c file.
//
// The image is either "raw" 1:1, or compressed using lzg algo.
//
#include <string.h>

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "lzg.h"

// this needs to be same as the fat16 image, or expect bad things
#define SECTOR 512

extern const unsigned char diskimg[2361915];
extern const int diskimg_length;
void *ramdisk;

DSTATUS disk_status (BYTE pdrv)
{
	(void)pdrv;
	return 0;
}


DSTATUS disk_initialize (BYTE pdrv)
{
	(void)pdrv;

	if(ramdisk != NULL) {
		uart_printf("disk: already initialized\n");
		return 0;
	}

	lzg_uint32_t origsize = LZG_DecodedSize(diskimg, diskimg_length);

	if (origsize == 0) {
		uart_printf("disk: lzg header not found, assuming raw img\n");
		ramdisk = diskimg;
		return 0;
	}

	uart_printf("disk: image in lzg format\n");

	ramdisk = malloc(origsize);
	if (ramdisk == NULL) {
		uart_printf("disk: malloc failed %d\n", origsize);
	}

	uart_printf("disk: decompressing\n");
	lzg_uint32_t ret = LZG_Decode(diskimg, diskimg_length, ramdisk, origsize);
	if (ret != origsize) {
		uart_printf("disk: failed\n");
		return STA_NOINIT;
	}
	uart_printf("disk: done = %d\n", ret);


	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	(void)pdrv;
	memcpy(buff, ramdisk + sector * SECTOR, count * SECTOR);
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	(void)pdrv;
	memcpy(ramdisk + sector * SECTOR, buff, count * SECTOR);
	return 0;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	(void)pdrv;
	(void)cmd;
	(void)buff;
	return RES_OK;
}

