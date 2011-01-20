all:
	gcc-4.5 -Wall -g glopher.c -o glopher `pkg-config gio-2.0 gtk+-2.0 clutter-1.0 clutter-gtk-0.10 --cflags --libs`
	#./glopher
