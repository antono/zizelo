#
# Variables and default configuration
#
NAME    = zizelo
VERSION = 0.1

CC      = gcc
CFLAGS  = -ggdb -O0 -Wall -Wl,--export-dynamic -I.
LIBS    = gio-2.0 gtk+-3.0 clutter-1.0 clutter-gtk-1.0 gmodule-export-2.0
OBJ 	= page.o gophernet.o guri.o
HEADERS = page.h gophernet.h guri.h

#
# Building
#
all: $(OBJ)
	$(CC) $(CFLAGS) -g $(NAME).c -o $(NAME) $(OBJ) `pkg-config $(LIBS) --cflags --libs`

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) `pkg-config $(LIBS) --cflags --libs`

run: all
	@./$(NAME)

.PHONY: clean
.PHONY: tags


sc: $(OBJ)
	$(CC) $(CFLAGS) -g scroll.c -o scroll `pkg-config $(LIBS) --cflags --libs`

tags:
	@ctags -R .

clean:
	rm -rf $(NAME) $(OBJ) *.swp core
