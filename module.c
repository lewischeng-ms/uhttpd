/*
 * file: module.c
 * description: open specified modules and maintain a list to cache them
 * author: Lewis Cheng
 * date: 2011.09.14
 */
 
#include "uhttpd.h"

static struct module {
	struct module *link;
	char *path;
	void *handle;
	handler_type handler;
} head;

handler_type open_module(const char *path_buf) {
	// Search if exists.
	struct module *curr = head.link;
	while (curr != NULL) {
		if (!xstrcmp(path_buf, curr->path, MAX_PATH_LENGTH))
			return curr->handler;
		curr = curr->link;
	}

	// open
	void *handle = dlopen(path_buf, RTLD_NOW);
	if (handle == NULL)
		return NULL;
	
	// new module node
	struct module *new_module = xmalloc(sizeof(struct module));
	new_module->link = head.link;
	head.link = new_module;
	new_module->handle = handle;
	new_module->path = xmalloc(MAX_PATH_LENGTH);
	xstrcpy(new_module->path, path_buf, MAX_PATH_LENGTH);
	
	// get function
	new_module->handler = (handler_type)dlsym(new_module->handle, "handler");
	if (new_module->handler == NULL) {
		xfree(new_module->path);
		xfree(new_module);
		return NULL;
	}
	
	return new_module->handler;
}
