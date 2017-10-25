
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


static void mod_load(struct mod_header *mh)
{
	printd("loading %s %x %x\n", mh->name, (int)(MOD_ADDR + mh->off), (int)mh->size);
	if(shared->m4_state != M4_STATE_HALT) {
		shared->m4_state = M4_STATE_FADEOUT;
		while(shared->m4_state == M4_STATE_FADEOUT);
		shared->m4_state = M4_STATE_HALT;
	}
	flash_read(MOD_ADDR + mh->off, (void *)M4_RAM_BASE, mh->size);
	shared->m4_state = M4_STATE_FADEIN;
	Chip_RGU_TriggerReset(RGU_M3_RST);
}


static int on_load_name(struct mod_header *mh, void *ptr)
{
	const char *name = ptr;
	if(strcmp(mh->name, name) == 0) {
		mod_load(mh);
		return 1;
	}
	return 0;
}


int mod_load_name(const char *name)
{
	return mod_foreach(on_load_name, (char *)name);
}


static int on_load_id(struct mod_header *mh, void *ptr)
{
	int *id = ptr;
	if(mh->id == *id) {
		mod_load(mh);
		return 1;
	}
	return 0;
}


int mod_load_id(int id)
{
	return mod_foreach(on_load_id, &id);
}


static int on_list(struct mod_header *mh, void *ptr)
{
	struct cmd_cli *cli = ptr;
	cmd_printd(cli, "%08x %08x %08x %3d: %s\n", (int)mh->off, (int)mh->size, (int)mh->sum, mh->id, mh->name);
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

