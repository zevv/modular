
/*

Table 24. Boot image header description

Address          Name             Description                              size [bits]

5:0              AES_ACTIVE[1]    AES encryption active                    6
                                  0x25 (100101): AES encryption active
                                  0x1A (011010): AES encryption not active
                                  all other values: invalid image
7:6              HASH_ACTIVE[1]   Indicates whether a hash is used:        2
                                  00: reserved for non-secure parts.
                                  01: reserved
                                  10: reserved
                                  11: no hash is used
13:8             RESERVED         11...11 (binary)                         6
15:14            AES_CONTROL      Reserved for non-secure parts.           2
31:16            HASH_SIZE[3]     Size of image copied to internal SRAM at 16
                                  boot time.
95:32            HASH_VALUE       Reserved for non-secure parts.           64
127:96           RESERVED         11...11 (binary)                         32

*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

struct lpc_header {
	uint8_t aes_hash_active;
	uint8_t reserved1;
	uint16_t size;
	uint64_t hash_value;
	uint32_t reserved2;
} __attribute__ (( __packed__ ));


uint32_t calc_hash(uint8_t *buf, size_t len)
{
	size_t i;
	uint32_t sum = 0;
	for(i=0; i<len; i++) {
		sum += buf[i] + 1;
	}
	return sum;
}


int main(int argc, char **argv)
{
	int r;
	struct stat st;
	struct lpc_header hdr;

	memset(&hdr, 0xff, sizeof(hdr));

	if(argc != 2) {
		fprintf(stderr, "usage: lpcheader <FNAME>\n");
		exit(1);
	}

	char *fname = argv[1];

	r = stat(fname, &st);
	if(r != 0) {
		fprintf(stderr, "stat: %s\n", strerror(errno));
		exit(1);
	}

	void *bin = malloc(st.st_size);
	assert(bin);

	FILE *f = fopen(fname, "r+");
	if(f == NULL) {
		fprintf(stderr, "fopen: %s\n", strerror(errno));
		exit(1);
	}

	r = fread(bin, 1, st.st_size, f);
	if(r != st.st_size) {
		fprintf(stderr, "fread: %s\n", strerror(errno));
		exit(1);
	}

	hdr.aes_hash_active = 0xda;
	hdr.size = st.st_size;
	hdr.hash_value = calc_hash(bin, st.st_size);

	r = fseek(f, 0, SEEK_SET);
	if(r != 0) {
		fprintf(stderr, "fseek: %s\n", strerror(errno));
		exit(1);
	}

	r = fwrite(&hdr, 1, sizeof(hdr), f);
	if(r != sizeof(hdr)) {
		fprintf(stderr, "fwrite: %s\n", strerror(errno));
		exit(1);
	}

	r = fwrite(bin, 1, st.st_size, f);
	if(r != st.st_size) {
		fprintf(stderr, "fwrite: %s\n", strerror(errno));
		exit(1);
	}

	fclose(f);

	return 0;
}

