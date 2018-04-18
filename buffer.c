#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "buffer.h"

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

	b->edit.end = 0;
	b->edit.len = 0;
	b->edit.alloc = EDIT_ALLOC_STEP;
	b->edit.buf = malloc(b->edit.alloc);
	if (!b->edit.buf) goto err2;

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
	free(b.edit.buf);
	munmap(b.file.buf, b.file.len);
	flock(b.file.fd, LOCK_UN);
	close(b.file.fd);
}

int buf_view_extend(struct buffer *b) {
	bool eof = false;
	size_t n = EDIT_ALLOC_STEP;
	// Ensure we don't exceed the bounds of the file
	if (n > b->file.len - b->edit.end) {
		n = b->file.len - b->edit.end;
		eof = true;
	}

	if (b->edit.len + n > b->edit.alloc) {
		size_t new_alloc = b->edit.len + EDIT_ALLOC_STEP;
		printf("realloc: %lu -> %lu\n", b->edit.alloc, new_alloc);
		char *tmp = realloc(b->edit.buf, new_alloc);
		if (!tmp) return -1;
		b->edit.buf = tmp;
		b->edit.alloc = new_alloc;
	}

	memcpy(b->edit.buf + b->edit.len, b->file.buf + b->edit.end, n);
	b->edit.len += n;
	b->edit.end += n;
	return eof ? 1 : 0;
}

int buf_view_shrink(struct buffer *b, size_t decrement) {
	if (b->edit.len < decrement)
		return 1;

	// TODO: write removed section to file buffer

	b->edit.len -= decrement;
	b->edit.end -= decrement;

	size_t new_alloc = b->edit.len + EDIT_ALLOC_STEP;
	if (b->edit.alloc / 2 > new_alloc) {
		char *tmp = realloc(b->edit.buf, new_alloc);
		if (!tmp) return -1;
		b->edit.buf = tmp;
		b->edit.alloc = new_alloc;
	}

	return 0;
}

void buf_sync(struct buffer b) {
	msync(b.file.buf, b.file.len, MS_ASYNC);
}
