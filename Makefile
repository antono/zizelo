#
# Variables and default configuration
#
NAME    = glopher
VERSION = 0.1

CC      = gcc
CFLAGS  = -ggdb -O2 -Wall -I.
LIBS    = gio-2.0 gtk+-2.0 clutter-1.0 clutter-gtk-0.10
OBJ 	= gophermap.o gophernet.o guri.o
HEADERS = gophermap.h gophernet.h guri.h


#
# Building
#
all: $(OBJ)
	$(CC) $(CFLAGS) -g $(NAME).c -o $(NAME) $(OBJ) `pkg-config $(LIBS) --cflags --libs`

run: all
	@./glopher

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) `pkg-config $(LIBS) --cflags --libs`

.PHONY: clean

clean:
	rm -rf *.o *.swp core
