#ifndef SYSCALLS_h_
#define SYSCALLS_h_

#define SYSCALL_DEVNAME       8

struct devoptab_t {
	const char name[SYSCALL_DEVNAME];
	int  (*init)(void);
	int  (*open_r )(struct _reent *r, const char *path, int flags, int mode);
	int  (*close_r)(struct _reent *r, int fd);
	long (*write_r)(struct _reent *r, int fd, const char *ptr, int len);
	long (*read_r )(struct _reent *r, int fd, char *ptr, int len);
	int  (*lseek_r)(struct _reent *r, int fd, int ptr, int dir);
	int  (*unlink_r)(struct _reent *r, const char *path);
};

void syscalls_init(void);

#endif
