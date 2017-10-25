
#define _GNU_SOURCE /* basename */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bsd/string.h>


struct mod_header {
	char name[32];
	uint32_t id;
	uint32_t off;
	uint32_t size;
	uint32_t sum;
};


struct blob {
	uint8_t *buf;
	size_t size;
	uint32_t sum;
};


static void die(const char *fmt, ...);
static struct blob *blob_read(const char *fname);
static void blob_calc_sum(struct blob *blob);
static struct blob *blob_alloc(size_t sizen);
static void blob_emit(struct blob *blob);

#define MAGIC 0x30444f4d /* MOD0 */


int main(int argc, char **argv)
{
	uint32_t magic = MAGIC;
	size_t count = argc-1;
	size_t id = 0;

	struct mod_header mod_header[count+1];
	memset(&mod_header, 0, sizeof(mod_header));

	struct blob *mod_blob[count];

	uint32_t off = sizeof(magic) + sizeof(mod_header);

	size_t i;
	for(i=0; i<count; i++) {

		const char *fname = argv[i+1];

		char name[32];
		snprintf(name, sizeof(name), "%s", basename(fname));
		char *p = strchr(name, '.');
		assert(p);
		*p = '\0';

		struct blob *blob = blob_read(fname);
		
		struct mod_header *h = &mod_header[i];
		snprintf(h->name, sizeof(h->name), "%s", name);
		h->sum = blob->sum;
		h->size = blob->size;
		h->off = off;
		h->id = id++;
		off += blob->size;

		mod_blob[i] = blob;

		fprintf(stderr, "- %08x %08x %s\n", h->off, h->size, h->name);
	}

	fwrite(&magic, 1, sizeof(magic), stdout);
	fwrite(&mod_header, 1, sizeof(mod_header), stdout);
	for(i=0; i<count; i++) {
		blob_emit(mod_blob[i]);
	}

	return 0;
}


static void die(const char *fmt, ...)
{
	int e = errno;
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, ": %s\n", strerror(e));
	va_end(va);
	exit(1);
}


static struct blob *blob_alloc(size_t size)
{
	struct blob *blob;
	blob = calloc(1, sizeof *blob);
	assert(blob);
	blob->buf = calloc(1, size);
	assert(blob->buf);
	blob->size = size;
	return blob;
}


static void blob_calc_sum(struct blob *blob)
{
	size_t i;
	blob->sum = 0;
	for(i=0; i<blob->size; i++) {
		blob->sum += blob->buf[i] + 1;
	}
}


static struct blob *blob_read(const char *fname)
{
	FILE *f = fopen(fname, "rp");
	if(f == NULL) {
		die("fopen");
	}

	fseek(f, 0, SEEK_END);
	struct blob *blob = blob_alloc(ftell(f));
	fseek(f, 0, SEEK_SET);

	int r = fread(blob->buf, 1, blob->size, f);
	if(r != blob->size) {
		die("fread");
	}

	fclose(f);

	blob_calc_sum(blob);

	return blob;
}


static void blob_emit(struct blob *blob)
{
	fwrite(blob->buf, 1, blob->size, stdout);
	free(blob->buf);
	free(blob);
}


/*
 * End
 */
