/*
 * file: url.c
 * description: parse URL and argument string
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

int parse_url(const char *url, char *path_buf, char *arg_str_buf, char *suffix_buf) {
	assert(url != NULL);
	assert(path_buf != NULL);
	assert(arg_str_buf != NULL);
	assert(suffix_buf != NULL);

	// Document root.
	if (*url++ != '/')
		return -1;
	
	// Parse path.
	xstrcpy(path_buf, document_root, MAX_PATH_LENGTH);
	url = copy_to_stop_char(path_buf, xstrlen(path_buf, MAX_PATH_LENGTH), url, '?', MAX_PATH_LENGTH);
	if (*(url - 1) == '/')
		strncat(path_buf, default_html_name, MAX_PATH_LENGTH);
		
	// Parse suffix.
	xstrcpy(suffix_buf, strrchr(path_buf, '.') + 1, MAX_SUFFIX_LENGTH);
	
	// Parse argument string.
	if (*url == '?') {
		copy_to_stop_char(arg_str_buf, 0, url + 1, '\0', MAX_ARGUMENT_STRING_LENGTH);
	} else if (*url == '\0') {
		empty_string(arg_str_buf);
	} else {
		return -1;
	}
	
	return 0;
}

static arg_node_type *create_arg_node(arg_node_type *link) {
	arg_node_type *node = xmalloc(sizeof(arg_node_type));
	node->link = link;
	node->key = NULL;
	node->value = NULL;
	return node;
}

arg_list_type create_arg_list(const char *arg_str) {
	arg_list_type list = create_arg_node(NULL);
	
	while (*arg_str != '\0') {
		arg_node_type *new_node = create_arg_node(list->link);
		list->link = new_node;
		
		// Parse key.
		new_node->key = xstralloc(MAX_KEY_VALUE_LENGTH);
		arg_str = copy_to_stop_char(new_node->key, 0, arg_str, '=', MAX_KEY_VALUE_LENGTH);
		
		if (*arg_str == '=') {
			++arg_str; // Skip '='.
		} else {
			free_arg_list(list);
			return NULL;
		}
		
		// Parse value.
		new_node->value = xstralloc(MAX_KEY_VALUE_LENGTH);
		arg_str = copy_to_stop_char(new_node->value, 0, arg_str, '&', MAX_KEY_VALUE_LENGTH);
		
		if (*arg_str == '&')
			++arg_str; // Skip '&'.
	}
	
	return list;
}

void free_arg_list(arg_list_type list) {
	assert(list != NULL);

	arg_node_type *curr = list->link;
	while (curr != NULL) {
		xfree(curr->key);
		xfree(curr->value);
		arg_node_type *next = curr->link;
		xfree(curr);
		curr = next;
	}
	
	xfree(list);
}
