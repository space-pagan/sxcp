PREFIX ?= /usr/local

CC  	:= g++ -Wall -g
CFLAGS  := -lX11 -std=c++11

.PHONY: all clean install uninstall

all: sxcp

sxcp: sxcp.cpp config.def.h config.h
	$(CC) -o $@ $< $(CFLAGS)

config.h:
	cp config.def.h $@

clean:
	@${RM} -vf *.o sxcp

install: sxcp
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 0755 sxcp $(DESTDIR)$(PREFIX)/bin/sxcp

uninstall:
	@${RM} -vf $(DESTDIR)$(PREFIX)/bin/sxcp
