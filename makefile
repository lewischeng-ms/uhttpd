# Configuration.
CFLAGS		= -Wall -g
SOURCES		= xmalloc.c log.c xrw.c xstring.c server.c mime.c url.c module.c http.c main.c
OBJECTS 	= $(SOURCES:.c=.o)
MODULES	= test.so

# Rules.
.PHONY: all clean

all: uhttpd $(MODULES)

clean:
	rm -f $(OBJECTS) $(MODULES) uhttpd

uhttpd: $(OBJECTS)
	$(CC) $(CFLAGS) -Wl,-export-dynamic -o $@ $^ -ldl

$(OBJECTS): uhttpd.h

$(MODULES): \
%.so: %.c uhttpd.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<
