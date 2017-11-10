
all: lpcsum lpchdr mkimg

lpchdr: lpchdr.c
	gcc -Wall -Werror lpchdr.c -o lpchdr

lpcsum: lpcsum.c
	gcc -Wall -Werror lpcsum.c -o lpcsum

mkimg: mkimg.c
	gcc -Wall -Werror mkimg.c -o mkimg -lbsd

midimon: midimon.c
	gcc -Wall -Werror midimon.c -o midimon -lasound

modcli: modcli.c
	gcc -Wall -Werror modcli.c -o modcli $(shell pkg-config --cflags --libs libusb-1.0)

clean:
	rm -f lpchdr lpcsum mkimg

