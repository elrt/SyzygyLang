CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

SRCDIR = src
BINDIR = bin
TARGET = syzygy

SOURCES = main.c parser.c lexer.c
OBJECTS = $(SOURCES:%.c=$(BINDIR)/%.o)

$(shell mkdir -p $(BINDIR))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/parser.h
$(BINDIR)/parser.o: $(SRCDIR)/parser.c $(SRCDIR)/parser.h $(SRCDIR)/lexer.h
$(BINDIR)/lexer.o: $(SRCDIR)/lexer.c $(SRCDIR)/lexer.h

clean:
	rm -f $(OBJECTS) $(TARGET)
	rmdir $(BINDIR) 2>/dev/null || true

.PHONY: all clean
