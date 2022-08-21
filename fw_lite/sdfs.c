
#include "platform.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ff.h"
#include "syscalls.h"
#include "sdfs.h"

#define SDFS_FDS  5

#define DEBUG 1

FATFS sdfs_fs;
FIL sdfs_fds[SDFS_FDS];

static BYTE flags2mode(int flags)
{
	BYTE mode;
	int accmode;

	mode = 0;
	accmode = flags & O_ACCMODE;

	if ((accmode == O_RDONLY) || (accmode == O_RDWR)) {
		mode |= FA_READ;
	}

	if ((accmode == O_WRONLY) || (accmode == O_RDWR)) {
		mode |= FA_WRITE;

		if (!(flags & O_CREAT)) {
			mode |= FA_OPEN_EXISTING;
		} else if (flags & O_EXCL) {
			mode |= FA_CREATE_NEW;
		} else if (flags & O_TRUNC) {
			mode |= FA_CREATE_ALWAYS;
		} else {
			mode |= FA_OPEN_ALWAYS;
		}
	}

	return mode;
}

int sdfs_fd_used(int fd)
{
	if (sdfs_fds[fd].obj.fs == NULL) {
		return 0;
	}

	return 1;
}

static int sdfs_init()
{
	int ret = 0;

	FRESULT result = f_mount(&sdfs_fs, "", 0);
#ifdef DEBUG
	printf("sdfs: f_mount:%d\n", result);
#endif
	if (result != FR_OK) {
		return -1;
	}

#ifdef DEBUG
	{
		FATFS *fs;
		DWORD fre_clust, fre_sect, tot_sect;

		if (f_getfree("", &fre_clust, &fs) != FR_OK) {
			printf("sdfd: failed to get free\n");
		}
		tot_sect = (fs->n_fatent - 2) * fs->csize;
		fre_sect = fre_clust * fs->csize;
		uart_printf("%d KiB total drive space.\n%d KiB available.\n", tot_sect / 2, fre_sect / 2);
	}
#endif

	return ret;
}

static int sdfs_open(struct _reent *r, const char *path, int flags, int mode)
{
	(void)r;
	(void)mode;
	unsigned int fd;
	
	for (fd = 0; fd < ARRAY_SIZE(sdfs_fds); fd++) {
		if (sdfs_fd_used(fd) == 0) {
			break;
		}
	}

	if (fd == ARRAY_SIZE(sdfs_fds)) {
		return -1;
	}

#if DEBUG
	printf("sdfs: Open '%s' flags 0x%x\n", path, flags);
#endif

	BYTE m = flags2mode(flags);
	FRESULT result = f_open(&sdfs_fds[fd], path, m);

	if (result != FR_OK) {
		return -1;
		//errno = fresult2errno(result);
	}

	return fd;
}

static int sdfs_close(struct _reent *r, int fd)
{
	(void)r;

	if (sdfs_fd_used(fd) == 0) {
		return -1;
	}

#if DEBUG
	printf("sdfs: close %d\n", fd);
#endif

	FRESULT result = f_close(&sdfs_fds[fd]);
	if (result != FR_OK) {
		//errno = fresult2errno(result);
		return -1;
	}

	return 0;
}

static long sdfs_write(struct _reent *r, int fd, const char *ptr, int len)
{
	(void)r;

	if (sdfs_fd_used(fd) == 0) {
		return -1;
	}

	UINT written;
	FRESULT result = f_write(&sdfs_fds[fd], ptr, len, &written);
	if (result != FR_OK) {
		//errno = fresult2errno(result);
		return -1;
	}

	return written;
}

static long sdfs_read(struct _reent *r, int fd, char *ptr, int len)
{
	(void)r;

	if (sdfs_fd_used(fd) == 0) {
		return -1;
	}

	UINT nbytes_read;
	FRESULT result = f_read(&sdfs_fds[fd], ptr, len, &nbytes_read);
	if (result != FR_OK) {
		//errno = fresult2errno(result);
		return -1;
	}

	return nbytes_read;
}

static int sdfs_lseek(struct _reent *r, int fd, int ptr, int dir)
{
	(void)r;

	if (sdfs_fd_used(fd) == 0) {
		return -1;
	}

	DWORD pos;

	switch (dir) {
		case SEEK_SET:
			pos = 0;
			break;
		case SEEK_CUR:
			pos = f_tell(&sdfs_fds[fd]);
			break;
		case SEEK_END:
			pos = f_size(&sdfs_fds[fd]);
			break;
		default:
			return -1;
	}

	pos += ptr;

	FRESULT result = f_lseek(&sdfs_fds[fd], pos);
	if (result != FR_OK) {
		//errno = fresult2errno(result);
		return -1;
	}

	return pos;
}

static int sdfs_unlink(struct _reent *r, const char *path)
{
	(void)r;
	FRESULT result = f_unlink(path);

	if (result != FR_OK) {
		//errno = fresult2errno(result);
		return -1;
	}

	return 0;
}

struct devoptab_t sdfs_devoptab = {
	"sdfs",
	sdfs_init,
	sdfs_open,
	sdfs_close,
	sdfs_write,
	sdfs_read,	
	sdfs_lseek,
	sdfs_unlink,
};
