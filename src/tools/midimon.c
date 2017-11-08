#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <termios.h>
#include <poll.h>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <signal.h>

static void bye(void);
static int set_noncanonical(int fd, struct termios *save);
static void parse_midi(uint8_t *buf, size_t n);

static struct termios save;


int main(int argc, char** argv)
{
	if(isatty(0)) {
		set_noncanonical(0, &save);
	}

	snd_rawmidi_t *handle_in = 0, *handle_out = 0;
	int err;
	
	err = snd_rawmidi_open(&handle_in, &handle_out, "virtual", SND_RAWMIDI_NONBLOCK);	
	if (err) {
		fprintf(stderr, "snd_rawmidi_open 'virtual' failed: %d\n", err);
	}

	int nfds = 1 + snd_rawmidi_poll_descriptors_count(handle_in);
	struct pollfd pfd[nfds];
	memset((void *)pfd, 0, sizeof(pfd));

	usleep(500000);

	pfd[0].events = POLLIN;
	pfd[0].fd = 0;

	snd_rawmidi_poll_descriptors(handle_in, pfd+1, nfds);
	
	for(;;) {
		int r = poll(pfd, nfds, 1000);

		if(r > 0) {
		
			if((pfd[0].revents & POLLIN) || (pfd[0].revents & POLLHUP)) {

				uint8_t buf[64];
				int r = read(0, buf+1, sizeof(buf)-2);
				if(r <= 0) {
					exit(0);
				}
				if(buf[1] == 3 || buf[1] == 4) {
					exit(0);
				}
				buf[0] = 0xf0;
				buf[r+1] = 0xf7;

				snd_rawmidi_write(handle_out, buf, r+2);

			}
			
			unsigned short revents;
			snd_rawmidi_poll_descriptors_revents(handle_in, &pfd[1], nfds - 1, &revents);

			if(revents & POLLIN) {
				uint8_t buf[256];
				ssize_t n = snd_rawmidi_read(handle_in, buf, sizeof(buf));
				if(n > 0) {
					parse_midi(buf, n);
				}
			}
		}
		usleep(1000);
	}

	return 0;
}


static void parse_midi(uint8_t *buf, size_t n)
{
	int i;
	bool in_sysex = false;
	for(i=0; i<n; i++) {
		uint8_t c = *buf++;

		if(c == 0xf0) {
			in_sysex = true;
		} else if(c == 0xf7) {
			in_sysex = false;
		} else {
			if(in_sysex) {
				fprintf(stdout, "%c", c);
				fflush(stdout);
			} else {
				if(c & 0x80) printf("\n");
				fprintf(stdout, "%02x ", c);
				fflush(stdout);
			}
		}
	}
}


static void bye(void)
{

	tcsetattr (0, TCSANOW, &save);
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

/*
 * End
 */

