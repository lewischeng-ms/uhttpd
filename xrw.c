/*
 * file: xrw.c
 * description: extension to system call read() and write() to detect EINTR
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

ssize_t xwrite(int fd, void *buf, size_t count) {
	assert(buf != NULL);

	ssize_t bytes_write;
	size_t bytes_left = count;
	char *ptr = (char *)buf;
	while (bytes_left > 0) {
		bytes_write = write(fd, ptr, bytes_left);
		if (bytes_write < 1) {
			// either EINTR or other error
			if (errno == EINTR) {
				// the call was interrupted before writing any data
				bytes_write = 0;
			} else {
				return -1;
			}
		}
		ptr += bytes_write;
		bytes_left -= bytes_write;
	}
	
	return count;
}

ssize_t xread(int fd, void *buf, size_t max) {
	assert(buf != NULL);
	
	ssize_t bytes_read = read(fd, buf, max);
	while (bytes_read == -1) {
		if (errno == EINTR)
			bytes_read = read(fd, buf, max);
		else
			return -1;
	}
	
	return bytes_read;
}
