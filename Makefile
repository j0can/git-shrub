CC = gcc
CFLAGS = -Wall -Wextra

all: git-shrub

git-shrub: shrub.c
	$(CC) $(CFLAGS) -o git-shrub shrub.c

install: git-shrub
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp git-shrub $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/git-shrub

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/git-shrub

clean:
	rm -f git-shrub

.PHONY: all install uninstall clean
