#include <error.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <poll.h>
#include <sys/time.h>

#define PAGE_SIZE 528

struct rpc {
	int fd;
	int id;
	struct pollfd pfd;
};


static void usage(void);
struct rpc *rpc_open(const char *ifname, int id);
static void rpc_do(struct rpc *rpc, int function, void *data, size_t len);

void cmd_m4_load(struct rpc *rpc, const char *fname);
void cmd_flash_image(struct rpc *rpc, const char *fname, uint32_t address);

void rpc_set_led(struct rpc *rpc, bool onoff);
void rpc_reboot(struct rpc *rpc);
void rpc_flash_set_load_addr(struct rpc *rpc, uint32_t addr);
void rpc_flash_load_data(struct rpc *rpc, void *data, size_t len);
void rpc_flash_write(struct rpc *rpc, uint32_t addr);
void rpc_mem_set_load_addr(struct rpc *rpc, uint32_t addr);
void rpc_mem_load_data(struct rpc *rpc, void *data, size_t len);
void rpc_m4_cmd(struct rpc *rpc, char cmd);

static void die(const char *fmt, ...);
static void debug(const char *fmt, ...);

static int opt_verbose = false;


int main(int argc, char **argv)
{
	char *opt_dev = "slcan0";
	int opt_id = 1;
	int c;

	while((c = getopt(argc, argv, "d:hi:v")) != -1) {
		switch(c) {
			case 'd':
				opt_dev = optarg;
				break;
			case 'h':
				usage();
				exit(0);
			case 'i':
				opt_id = atoi(optarg);
				break;
			case 'v':
				opt_verbose = true;
				break;
			case '*':
				usage();
				exit(1);
		}
	}
	
	argc -= optind; argv += optind;

	if(argc < 1) {
		usage();
		exit(1);
	}
	
	struct rpc *rpc = rpc_open(opt_dev, opt_id);

	const char *cmd = argv[0];
	argc --; argv ++;

	if(strcmp(cmd, "flash") == 0) {
		if(argc != 2) {
			usage();
			exit(1);
		}
		cmd_flash_image(rpc, argv[0], atoi(argv[1]));
		return 0;
	}
	
	if(strcmp(cmd, "load") == 0) {
		if(argc != 1) {
			usage();
			exit(1);
		}
		cmd_m4_load(rpc, argv[0]);
		return 0;
	}

	usage();
	return 1;
}


static void usage(void)
{
	fprintf(stderr, 
		"usage: modrpc [-d dev] [-i id] <cmd> [args]\n"
		"\n"
		"commands:\n"
		"   flash <fname> <addr>\n"
	);
}


static void flash_page(struct rpc *rpc, uint32_t addr, void *data, size_t len)
{
	uint32_t todo = len;

	debug("flash @ %08x\n", addr);

	rpc_flash_set_load_addr(rpc, 0);

	while(todo > 0) {
		uint32_t n = todo < 7 ? todo : 7;
		rpc_flash_load_data(rpc, data, n);
		data += n;
		todo -= n;
	}

	rpc_flash_write(rpc, addr);
}


/*
 * Command handlers
 */

void cmd_flash_image(struct rpc *rpc, const char *fname, uint32_t addr)
{
	FILE *f;

	f = fopen(fname, "rb");
	if(f == NULL) die("fopen: %s", strerror(errno));

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t *img = malloc(size);
	assert(img);
	fread(img, 1, size, f);
	
	fclose(f);

	uint32_t todo = size;
	uint32_t off = 0;

	while(todo > 0) {
		uint32_t n = todo < PAGE_SIZE ? todo : PAGE_SIZE;
		flash_page(rpc, addr, img+off, n);
		addr += n;
		todo -= n;
		off += n;
	}
}


void cmd_m4_load(struct rpc *rpc, const char *fname)
{
	FILE *f;

	f = fopen(fname, "rb");
	if(f == NULL) die("fopen: %s", strerror(errno));

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t *img = malloc(size);
	assert(img);
	fread(img, 1, size, f);
	
	fclose(f);

	uint32_t todo = size;
	uint32_t off = 0;
	
	rpc_m4_cmd(rpc, 'h');
	usleep(100000);
	
	rpc_mem_set_load_addr(rpc, 0x10000000);

	while(todo > 0) {
		uint32_t n = todo < 7 ? todo : 7;
		rpc_mem_load_data(rpc, img+off, n);
		todo -= n;
		off += n;
	}

	rpc_m4_cmd(rpc, 'g');
}


/*
 * RPC primitives
 */

void rpc_flash_set_load_addr(struct rpc *rpc, uint32_t addr)
{
	uint8_t data[4] = { addr >> 24, addr >> 16, addr >> 8, addr };
	rpc_do(rpc, 0x03, data, sizeof(data));
}


void rpc_flash_load_data(struct rpc *rpc, void *data, size_t len)
{
	assert(len <= 7);
	rpc_do(rpc, 0x04, data, len);
}


void rpc_flash_write(struct rpc *rpc, uint32_t addr)
{
	uint8_t data[4] = { addr >> 24, addr >> 16, addr >> 8, addr };
	rpc_do(rpc, 0x05, data, sizeof(data));
}


void rpc_mem_set_load_addr(struct rpc *rpc, uint32_t addr)
{
	uint8_t data[4] = { addr >> 24, addr >> 16, addr >> 8, addr };
	rpc_do(rpc, 0x06, data, sizeof(data));
}


void rpc_mem_load_data(struct rpc *rpc, void *data, size_t len)
{
	assert(len <= 7);
	rpc_do(rpc, 0x07, data, len);
}


void rpc_m4_cmd(struct rpc *rpc, char cmd)
{
	rpc_do(rpc, 0x08, &cmd, sizeof(cmd));
}


void rpc_set_led(struct rpc *rpc, bool onoff)
{
	uint8_t data = onoff;
	rpc_do(rpc, 0x01, &data, sizeof(data));
}


void rpc_reboot(struct rpc *rpc)
{
	rpc_do(rpc, 0x02, NULL, 0);
}


struct rpc *rpc_open(const char *ifname, int id)
{
	struct rpc *rpc;

	rpc = calloc(sizeof *rpc, 1);
	assert(rpc);

	struct ifreq ifr;
	struct sockaddr_can addr;
	int r;

	int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(fd < 0) die("socket: %s", strerror(errno));

	strcpy(ifr.ifr_name, ifname);
	r = ioctl(fd, SIOCGIFINDEX, &ifr);
	if(r < 0) die("SIOCGIFINDEX: %s", strerror(errno));

	addr.can_ifindex = ifr.ifr_ifindex;
	addr.can_family = AF_CAN;

	r = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	if(r != 0) die("bind: %s", strerror(errno));

	rpc->fd = fd;
	rpc->id = id;
	rpc->pfd.fd = fd;
	rpc->pfd.events = POLLIN;
	return rpc;
}


static void rpc_do(struct rpc *rpc, int function, void *data, size_t len)
{
	struct can_frame frame;

	assert(len <= 7);

	memset(&frame, 0, sizeof frame);

	frame.can_id = rpc->id;
	frame.can_dlc = len+1;
	frame.data[0] = function;

	memcpy(frame.data+1, data, len);

	int r = write(rpc->fd, &frame, sizeof frame);
	if(r != sizeof(frame)) die("write: %s", strerror(errno));

	debug("rpc %02x: ", function);
	size_t i;
	uint8_t *p = data;
	for(i=0; i<len; i++) debug("%02x ", *p++);
	debug("= ");

	r = poll(&rpc->pfd, 1, 1000);
	if(r < 0) die("poll: %s", strerror(errno));
	if(r == 1) {
		r = recv(rpc->fd, &frame, sizeof frame, 0);
		assert(frame.data[0] == function);
		debug("%d\n", frame.data[1]);
	} else {
		die("timeout");
	}
}


static void die(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
	exit(1);
}


static void debug(const char *fmt, ...)
{
	if(opt_verbose) {
		va_list va;
		va_start(va, fmt);
		vfprintf(stderr, fmt, va);
		va_end(va);
	}
}


/*
 * End
 */

