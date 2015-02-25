/*
 * file: xmalloc.c
 * description: extension to c library malloc() and free() to abort on failure and detect memory leakage
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

static int xmval = 0;

void *xmalloc(size_t size) {
	void *p = malloc(size);
	if (p == NULL)
		report_fatal("xmalloc()", "not enough memory");
	++xmval;

	return p;
}

void xfree(void *ptr) {
	if (ptr == NULL)
		return;
	--xmval;
	free(ptr);
}

int get_xmval() {
	return xmval;
}
