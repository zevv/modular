
all: lpcsum lpchdr mkimg

lpchdr: lpchdr.c
	gcc -Wall -Werror lpchdr.c -o lpchdr

lpcsum: lpcsum.c
	gcc -Wall -Werror lpcsum.c -o lpcsum

mkimg: mkimg.c
	gcc -Wall -Werror mkimg.c -o mkimg -lbsd

clean:
	rm -f lpchdr lpcsum mkimg

