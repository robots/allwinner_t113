#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include "FreeRTOS.h"
#include "task.h"

#include "syscalls.h"

#include "uart.h"

#define FDDEVSHIFT 8
#define FDMASK     0xff

static unsigned char* _cur_brk;
extern unsigned char __heap_start;
extern unsigned char __heap_end;

static struct devoptab_t con_devoptab;

const struct devoptab_t *syscall_devs[] = {
	&con_devoptab,
//	&sdfs_devoptab,
	NULL
};

void syscalls_init(void)
{
	uint32_t i = 0;

	while (syscall_devs[i]) {
		syscall_devs[i]->init();
		i++;
	}
}

static int path2dev(const char *path)
{
	char devname[SYSCALL_DEVNAME];
	int which_devoptab = 0;
	char *file;

	file = strstr(path, "/");
	if ((!file) || ((file - path) >= SYSCALL_DEVNAME)) {
		errno = ENODEV;
		return -1;
	}

	memset(devname, 0, SYSCALL_DEVNAME);
	memcpy(devname, path, file - path);

	do {
		if (strcmp(syscall_devs[which_devoptab]->name, devname) == 0) {
			break;
		}
		which_devoptab++;
	} while (syscall_devs[which_devoptab]);

	if (!syscall_devs[which_devoptab]) {
		errno = ENODEV;
		return -1;
	}

	return which_devoptab;
}

int _open_r(struct _reent *ptr, const char *path, int flags, int mode )
{
	int which_devoptab = 0;
	int fd = -1;
	char *file;

	file = strstr(path, "/");
	if ((!file) || ((file - path) >= SYSCALL_DEVNAME)) {
		errno = ENODEV;
		return -1;
	}
	file++;

	which_devoptab = path2dev(path);
	if (which_devoptab < 0) {
		errno = ENODEV;
		return -1;
	}

	fd = syscall_devs[which_devoptab]->open_r( ptr, file, flags, mode );

	if (fd == -1) {
		return -1;
	}

	fd |= (which_devoptab << FDDEVSHIFT);

	return fd;
}

long _close_r(struct _reent *ptr, int fd)
{
	int which;

	which = fd >> FDDEVSHIFT;
	fd = fd & FDMASK;

#ifdef DEBUG
	console_printf(CON_ERR, "SYS: close_r = dev: %d fd:%d \n", which, fd);
#endif

	return syscall_devs[which]->close_r(ptr, fd);
}

long _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt )
{
	int which;

	which = fd >> FDDEVSHIFT;
	fd = fd & FDMASK;

#ifdef DEBUG
	console_printf(CON_ERR, "SYS: write_r = dev: %d fd:%d \n", which, fd);
#endif

	return syscall_devs[which]->write_r(ptr, fd, buf, cnt);
}

long _read_r(struct _reent *ptr, int fd, void *buf, size_t cnt )
{
	int which;

	which = fd >> FDDEVSHIFT;
	fd = fd & FDMASK;

#ifdef DEBUG
	console_printf(CON_ERR, "SYS: read_r = dev: %d fd:%d \n", which, fd);
#endif

	return syscall_devs[which]->read_r(ptr, fd, buf, cnt);
}

int _lseek_r(struct _reent *r, int fd, int ptr, int dir)
{
	int which;

	which = fd >> FDDEVSHIFT;
	fd = fd & FDMASK;

#ifdef DEBUG
	console_printf(CON_ERR, "SYS: lseek_r = dev: %d fd:%d \n", which, fd);
#endif

	return syscall_devs[which]->lseek_r(r, fd, ptr, dir);
}

int _unlink_r(struct _reent *ptr, const char *path)
{
	int which = path2dev(path);
	char *file;

	if (which < 0) {
		return -1;
	}

	file = strstr(path, "/");
	if ((!file) || ((file - path) >= SYSCALL_DEVNAME)) {
		errno = ENODEV;
		return -1;
	}
	file++;

#ifdef DEBUG
	console_printf(CON_ERR, "SYS: unlink_r = dev: %d file:%s \n", which, file);
#endif

	return syscall_devs[which]->unlink_r(ptr, file);
}

int _isatty (int fd)
{
	int which;

	which = fd >> FDDEVSHIFT;

	if (which == 0) {
		return 1;
	}

	return 0;
}

int _fstat (int fd, struct stat *st)
{
	(void)fd;
	(void)st;
/*
	st->st_mode = S_IFCHR;
	st->st_blksize = 256;
	return 0;
*/
	return -1;
}

void __malloc_lock(struct _reent *r)
{
	(void)r;

  vTaskSuspendAll();
}

void __malloc_unlock(struct _reent *r)
{
	(void)r;

	xTaskResumeAll();
}

caddr_t _sbrk_r (struct _reent* r, int incr)
{
	(void)r;

	if (_cur_brk == NULL) {
		_cur_brk = &__heap_start;
	}

	unsigned char* _old_brk = _cur_brk;

	if ((uintptr_t)(_cur_brk + incr) > (uintptr_t)(&__heap_end))
	{
		uart_printf("heap full\n");

		errno = ENOMEM;
		return (void *)-1;
	}

	_cur_brk += incr;

	return (caddr_t)_old_brk;
}

void _exit (int rc)
{
	(void)rc;
	while (1)
	{
	}
}

int _kill (int pid, int sig)
{
	(void)pid;
	(void)sig;

	errno = EINVAL;

	return -1;
}

int _getpid ()
{
	return 1;
}

static int con_init()
{
	uart_init_dma();	

	return 0;
}

static int con_open(struct _reent *r, const char *path, int flags, int mode)
{
	(void)r;
	(void)path;
	(void)flags;
	(void)mode;

	return -1;
}

static int con_close(struct _reent *r, int fd)
{
	(void)r;
	(void)fd;

	return 0;
}

static long con_write(struct _reent *r, int fd, const char *ptr, int len)
{
	(void)r;

	switch (fd) {
		case STDOUT_FILENO:
			uart_send(ptr, len);
			break;
		case STDERR_FILENO:
			uart_send(ptr, len);
			break;
		default:
			return -1;
	}

	return len;
}

static long con_read(struct _reent *r, int fd, char *ptr, int len)
{
	(void)r;
	(void)fd;
	(void)ptr;
	(void)len;

	return -1;
}

static int con_lseek(struct _reent *r, int fd, int ptr, int dir)
{
	(void)r;
	(void)fd;
	(void)ptr;
	(void)dir;

	return -1;
}

static int con_unlink(struct _reent *r, const char *path)
{
	(void)r;
	(void)path;

	return -1;
}


static struct devoptab_t con_devoptab = {
	"console",
	con_init,
	con_open,
	con_close,
	con_write,
	con_read,
	con_lseek,
	con_unlink,
};
