
#include <stdlib.h>
#include <string.h>

#include "chip.h"
#include "cmd.h"
#include "printd.h"
#include "shared.h"
#include "flash.h"

#define M4_RAM_BASE 0x10000000
#define MOD_ADDR 0x21000

struct mod_header {
	char name[32];
	uint32_t id;
	uint32_t off;
	uint32_t size;
	uint32_t sum;
};


#define MAGIC 0x30444f4d /* MOD0 */


static int mod_foreach(int(*fn)(struct mod_header *mh, void *ptr), void *ptr)
{
	uint32_t addr = MOD_ADDR;
	uint32_t magic;
	int rv = 1;

	flash_read(addr, &magic, sizeof(magic));

	if(magic != MAGIC) {
		printd("wrong magic\n");
		return 1;
	}

	addr += sizeof(magic);

	for(;;) {
		struct mod_header mh;
		flash_read(addr, &mh, sizeof(mh));
		if(mh.off == 0) {
			break;
		}
		int rv2 = fn(&mh, ptr);
		if(rv2 == 0) rv = 0;
		addr += sizeof(mh);
	}
	return rv;
}


static int mod_load(struct mod_header *mh)
{
	printd("loading %s at %x size %x\n", mh->name, (int)(MOD_ADDR + mh->off), (int)mh->size);

	/* Ask M4 to shutdown */

	shared->m4_state = M4_STATE_HALT;
	{
		volatile int i;
		for(i=0; i<1000000; i++);
	}
	
	/* Load module from flash */

	flash_read(MOD_ADDR + mh->off, (void *)M4_RAM_BASE, mh->size);

	/* Verify checksum */

	uint32_t sum = 0;
	uint8_t *p = (void *)M4_RAM_BASE;
	size_t i;
	for(i=0; i<mh->size; i++) {
		sum += *p + 1;
		p++;
	}

	if(sum == mh->sum) {
		shared->m4_state = M4_STATE_FADEIN;
		Chip_RGU_TriggerReset(RGU_M3_RST);
	} else {
		printd("mod '%s' sum err: %08x != %08x\n", mh->name, sum != mh->sum);
		return 0;
	}

	return 1;
}


static int on_load_name(struct mod_header *mh, void *ptr)
{
	const char *name = ptr;
	if(strcmp(mh->name, name) == 0) {
		return mod_load(mh);
	}
	return 1;
}


int mod_load_name(const char *name)
{
	return mod_foreach(on_load_name, (char *)name);
}


static int on_load_id(struct mod_header *mh, void *ptr)
{
	uint32_t *id = ptr;
	if(mh->id == *id) {
		return mod_load(mh);
	}
	return 1;
}


int mod_load_id(int id)
{
	return mod_foreach(on_load_id, &id);
}


static int on_list(struct mod_header *mh, void *ptr)
{
	struct cmd_cli *cli = ptr;

	uint32_t sum = 0;
	uint32_t s = mh->size;
	uint32_t addr = MOD_ADDR + mh->off;
	while(s > 0) {
		uint32_t n = (s > 16) ? 16 : s;
		uint8_t buf[16];
		flash_read(addr, buf, n);
		size_t i;
		for(i=0; i<n; i++) {
			sum += buf[i] + 1;
		}
		s -= n;
		addr += n;
	}

	cmd_printd(cli, "%08x %08x %08x %3d: %s", (int)mh->off, (int)mh->size, (int)mh->sum, mh->id, mh->name);

	if(sum != mh->sum) {
		cmd_printd(cli, " (CRC err)");
	}

	cmd_printd(cli, "\n");

	return 1;
}


static int on_cmd_mod(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc == 1) {
		return mod_load_name(argv[0]);
	} else {
		return mod_foreach(on_list, cli);
	}
	return 0;
}


CMD_REGISTER(mod, on_cmd_mod, "");

