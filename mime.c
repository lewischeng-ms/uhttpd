/*
 * file: mime.c
 * description: provide look-up for file suffix and its corresponding mime type
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

static struct {
	char *suffix;
	char *mime_type;
} mime_table[] = {
	{ "asf",	"video/x-ms-asf" },
	{ "asx",	"video/x-ms-asf" },
	{ "au",		"audio/basic" },
	{ "avi",	"video/x-msvideo" },
	{ "bas",	"text/plain" },
	{ "bin",	"application/octet-stream" },
	{ "bmp",	"image/bmp" },
	{ "c",		"text/plain" },
	{ "cat",	"application/vnd.ms-pkiseccat" },
	{ "cdf",	"application/x-cdf" },
	{ "cer",	"application/x-x509-ca-cert" },
	{ "class",	"application/octet-stream" },
	{ "cpio",	"application/x-cpio" },
	{ "csh",	"application/x-csh" },
	{ "css",	"text/css" },
	{ "dll",	"application/x-msdownload" },
	{ "doc",	"application/msword" },
	{ "dot",	"application/msword" },
	{ "dvi",	"application/x-dvi" },
	{ "eps",	"application/postscript" },
	{ "exe",	"application/octet-stream" },
	{ "gif",	"image/gif" },
	{ "gz",		"application/x-gzip" },
	{ "h",		"text/plain" },
	{ "hlp",	"application/winhlp" },
	{ "hta",	"application/hta" },
	{ "htm",	"text/html" },
	{ "html",	"text/html" },
	{ "ico",	"image/x-icon" },
	{ "jpe",	"image/jpeg" },
	{ "jpeg",	"image/jpeg" },
	{ "jpg",	"image/jpeg" },
	{ "js",		"application/x-javascript" },
	{ "latex",	"application/x-latex" },
	{ "m3u",	"audio/x-mpegurl" },
	{ "mht",	"message/rfc822" },
	{ "mhtml",	"message/rfc822" },
	{ "mid",	"audio/mid" },
	{ "mov",	"video/quicktime" },
	{ "mp3",	"audio/mpeg" },
	{ "mpeg",	"video/mpeg" },
	{ "mpg",	"video/mpeg" },
	{ "pdf",	"application/pdf" },
	{ "pfx",	"application/x-pkcs12" },
	{ "png",	"image/png" },
	{ "pps",	"application/vnd.ms-powerpoint" },
	{ "ppt",	"application/vnd.ms-powerpoint" },
	{ "ps",		"application/postscript" },
	{ "ra",		"audio/x-pn-realaudio" },
	{ "ram",	"audio/x-pn-realaudio" },
	{ "rtf",	"application/rtf" },
	{ "sh",		"application/x-sh" },
	{ "swf",	"application/x-shockwave-flash" },
	{ "tar",	"application/x-tar" },
	{ "tex",	"application/x-tex" },
	{ "tgz",	"application/x-compressed" },
	{ "tif",	"image/tiff" },
	{ "tiff",	"image/tiff" },
	{ "txt",	"text/plain" },
	{ "wav",	"audio/x-wav" },
	{ "wmf",	"application/x-msmetafile" },
	{ "wps",	"application/vnd.ms-works" },
	{ "wri",	"application/x-mswrite" },
	{ "xls",	"application/vnd.ms-excel" },
	{ "xlt",	"application/vnd.ms-excel" },
	{ "xml",	"text/xml" },
	{ "xhtml",	"application/xhtml+xml" },
	{ "zip", 	"application/zip" },
	{ NULL,		"application/octet-stream" }
};


char *get_mime_type(char *suffix) {
		int i = 0;
		while (1) {
			if (mime_table[i].suffix == NULL || !xstrcmp(suffix, mime_table[i].suffix, MAX_SUFFIX_LENGTH))
				break;
			++i;
		}
		return mime_table[i].mime_type;
}
