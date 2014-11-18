CFLAGS=`pkg-config --cflags gtk+-3.0 libvlc`
LIBS=`pkg-config --libs gtk+-3.0 libvlc`

all:
	gcc -o vlcgtk3_0 vlcgtk3_0.c -Wall ${LIBS} ${CFLAGS}
clean:
	rm vlcgtk3_0
