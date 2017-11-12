
all: lpcsum lpchdr mkimg modcli

lpchdr: lpchdr.c
	gcc -Wall -Werror lpchdr.c -o lpchdr

lpcsum: lpcsum.c
	gcc -Wall -Werror lpcsum.c -o lpcsum

mkimg: mkimg.c
	gcc -Wall -Werror mkimg.c -o mkimg -lbsd

modcli: modcli.c
	gcc -Wall -Werror modcli.c -o modcli $(shell pkg-config --cflags --libs libusb-1.0)

modrpc: modrpc.c
	gcc -Wall -Werror modrpc.c -o modrpc

clean:
	rm -f lpchdr lpcsum mkimg

