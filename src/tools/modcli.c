#include <stdio.h>
#include <libusb.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>

static int set_noncanonical(int fd, struct termios *save);
static void bye(void);

static struct termios save;

int main(int argc, char **argv)
{
	libusb_context *ctx;
	libusb_init(&ctx);
	
	if(isatty(0)) {
		set_noncanonical(0, &save);
	}

	struct libusb_device_handle *h = libusb_open_device_with_vid_pid (ctx, 0x198c, 0x1c0d); 

	if(h == NULL) {
		fprintf(stderr, "Device not found\n");
		return 1;
	}

	int r;

	r = libusb_claim_interface(h, 2);
	if(r != 0) {
		fprintf(stderr, "Error claiming interface: %s\n", strerror(-r));
		return 1;
	}

	/* Stdin -> nonblocking */
	
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

	for(;;) {

		uint8_t buf[512];

		r = read(0, buf, sizeof(buf));

		if(r < 0 && errno != EAGAIN) {
			fprintf(stderr, "read(): %s\n", strerror(errno));
			exit(1);
		} else if(r > 0) {
			if(buf[0] == 3 || buf[0] == 4) {
				exit(0);
			}
			int len;
			libusb_bulk_transfer(h, 0x02, buf, r, &len, 100);
		} else if(r == 0) {
			exit(0);
		}
		
		int len;
		r = libusb_bulk_transfer(h, 0x82, buf, sizeof(buf), &len, 10);

		if(r == 0) {
			write(1, buf, len);
		} else if(r == LIBUSB_ERROR_NO_DEVICE) {
			fprintf(stderr, "[disconnect]\n");
			exit(0);
		}
	}

	return 0;
}


static int set_noncanonical(int fd, struct termios *save)
{
	int r;
	struct termios tios;
	
	if(save) tcgetattr(fd, save);
	tcgetattr(fd, &tios);

	tios.c_lflag     = 0;
	tios.c_cc[VTIME] = 0;
	tios.c_cc[VMIN]  = 1;

	tcflush (fd, TCIFLUSH);
	r = tcsetattr (fd, TCSANOW, &tios);
	if(r != 0) printf("tcsetattr : %s\n", strerror(errno));
	
	atexit(bye);

	return(0);
}


static void bye(void)
{
	tcsetattr (0, TCSANOW, &save);
}

/*
 * End
 */
