#define _DEFAULT_SOURCE // For MAP_ANONYMOUS
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "buffer.h"

#define EDIT_ALLOC 4 * (1<<10) // 4KiB

int buf_init(struct buffer *b, char *filename) {
	int errno_bak;

	b->file.fd = open(filename, O_RDWR | O_CREAT, 0666);
	if (b->file.fd < 0) return -1;
	if (flock(b->file.fd, LOCK_EX | LOCK_NB)) goto err0;

	struct stat s;
	if (fstat(b->file.fd, &s)) goto err1;
	b->file.len = s.st_size;

	b->file.buf = mmap(NULL, b->file.len, PROT_READ | PROT_WRITE, MAP_SHARED, b->file.fd, 0);
	if (b->file.buf == MAP_FAILED) goto err1;

	b->edit.start = 0;
	b->edit.end = 0;
	b->edit.len = 0;
	b->edit.buf = mmap(NULL, EDIT_ALLOC, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (b->edit.buf == MAP_FAILED) goto err2;

	b->damage_cb = NULL;
	b->damage_data = NULL;

	return 0;

err2:
	errno_bak = errno;
	munmap(b->file.buf, b->file.len);
	errno = errno_bak;
err1:
	errno_bak = errno;
	flock(b->file.fd, LOCK_UN);
	errno = errno_bak;
err0:
	errno_bak = errno;
	close(b->file.fd);
	errno = errno_bak;
	return -1;
}

void buf_free(struct buffer b) {
	munmap(b.edit.buf, EDIT_ALLOC);
	munmap(b.file.buf, b.file.len);
	flock(b.file.fd, LOCK_UN);
	close(b.file.fd);
}

void buf_sync(struct buffer b) {
	msync(b.file.buf, b.file.len, MS_ASYNC);
}
