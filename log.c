/*
 * file: log.c
 * description: for reporting general info, error and fatal error
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

static FILE *strm;

static char *get_time_string() {
	time_t t = time(NULL);
	return ctime(&t);
}

void open_logfile(const char *name) {
	if (name != NULL)
		strm = fopen(name, "a+");
	if (strm == NULL)
		strm = stdout;
}

static void close_logfile() {
	if (strm == NULL || strm == stdout)
		return;
	fclose(strm);
}

void report_fatal(const char *where, const char *format, ...) {
	va_list args;
	va_start(args, format);
	
	fprintf(strm, "%sFatal: %s: ", get_time_string(), where);
	vfprintf(strm, format, args);
	fprintf(strm, "\n\n");
	
	va_end(args);
	close_logfile();
	exit(1);
}

void report_error(const char *where, const char *format, ...) {
	va_list args;
	va_start(args, format);
	
	fprintf(strm, "%sError: %s: ", get_time_string(), where);
	vfprintf(strm, format, args);
	fprintf(strm, "\n\n");
	
	va_end(args);
}

void report_info(const char *where, const char *format, ...) {
	va_list args;
	va_start(args, format);
	
	fprintf(strm, "%sInfo: %s: ", get_time_string(), where);
	vfprintf(strm, format, args);
	fprintf(strm, "\n\n");
	
	va_end(args);
}
