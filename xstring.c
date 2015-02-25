/*
 * file: xstring.c
 * description: provide extension to c library string functions to ensure extra safety
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

char *xstrcpy(char *dest, const char *src, int max) {
	assert(dest != NULL);
	assert(src != NULL);
	assert(max > -1);
	
	int i = 0;
	while (i < max && *src != '\0')
		dest[i++] = *src++;
	dest[i] = '\0';
	
	return dest;
}

const char *copy_to_stop_char(char *dest, int start, const char *src, char stop_ch, int max) {
	assert(dest != NULL);
	assert(start > -1 && start < max);
	assert(src != NULL);
	assert(max > -1);
	
	while (start < max && *src != '\0' && *src != stop_ch)
		dest[start++] = *src++;
	dest[start] = '\0';
		
	return src;
}

char *empty_string(char *str) {
	assert(str != NULL);
	
	*str = '\0';
	
	return str;
}

int xstrlen(const char *str, int max) {
	assert(str != NULL);
	assert(max > -1);
	
	int len = 0;
	while (len <= max && *str != '\0') {
		++len;
		++str;
	}
	
	return len;
}

int xstrcmp(const char *s1, const char *s2, int max) {
	int i = 0;
	while (i < max && *s1 != '\0' && *s2 != '\0') {
		if (*s1 < *s2)
			return -1;
		else if (*s1 > *s2)
			return 1;
		++i;
		++s1;
		++s2;
	}
	if (*s1 != '\0')
		return 1;
	if (*s2 != '\0')
		return -1;
	return 0;
}

char *xstralloc(int max) {
	assert(max > -1);
	
	char *str = xmalloc(max + 1);
	
	return str;
}
