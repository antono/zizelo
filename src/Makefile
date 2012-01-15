EXE	= zizelo
SRC	= zizelo.vala page.vala net.vala actions.vala view.vala
FLAGS	= --pkg gtk+-3.0 --pkg gmodule-2.0 --pkg gee-1.0 \
	--pkg clutter-1.0 --pkg clutter-gtk-1.0 \
	--pkg gnet-2.0 --pkg gio-2.0  \
	--enable-experimental

all: bin
	@./$(EXE)

bin:
	valac $(FLAGS) $(SRC) --output=$(EXE)

clean:
	rm $(EXE)
