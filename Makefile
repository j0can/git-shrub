CC = gcc
CFLAGS = -Wall -Wextra -O2
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

TARGET = git-shrub
SRCDIR = src
BUILDDIR = build

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

test:
	@echo "Running tests..."
	@# Add test commands here when tests are implemented
