
#include <stdint.h>
#include <string.h>

#include "cmd.h"
#include "printd.h"
#include "spifilib_api.h"
#include "xmodem.h"


#ifndef SPIFLASH_BASE_ADDRESS
#define SPIFLASH_BASE_ADDRESS (0x14000000)

#endif

static SPIFI_HANDLE_T *spifi = NULL;
static uint32_t lmem[21];


static const PINMUX_GRP_T spifipinmuxing[] = {
        {0x3, 3,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},   /* SPIFI CLK */
        {0x3, 4,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},   /* SPIFI D3 */
        {0x3, 5,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},   /* SPIFI D2 */
        {0x3, 6,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},   /* SPIFI D1 */
        {0x3, 7,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},   /* SPIFI D0 */
        {0x3, 8,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}    /* SPIFI CS/SSEL */
};


void spifi_init(void)
{
	uint32_t memSize;
	Chip_SCU_SetPinMuxing(spifipinmuxing, sizeof(spifipinmuxing) / sizeof(PINMUX_GRP_T));

	spifiInit(LPC_SPIFI_BASE, true);

	spifiRegisterFamily(SPIFI_REG_FAMILY_SpansionS25FLP);
	spifiRegisterFamily(SPIFI_REG_FAMILY_SpansionS25FL1);
	spifiRegisterFamily(SPIFI_REG_FAMILY_MacronixMX25L);
	
	memSize = spifiGetHandleMemSize(LPC_SPIFI_BASE);
	if (memSize == 0) {
		return;
	}

	spifi = spifiInitDevice(&lmem, sizeof(lmem), LPC_SPIFI_BASE, SPIFLASH_BASE_ADDRESS);
	if(spifi == NULL) {
		return;
	}

	/* Setup SPIFI clock to at the maximum interface rate the detected device
	   can use. */

	uint32_t spifiBaseClockRate;
	uint32_t maxSpifiClock;

	spifiBaseClockRate = Chip_Clock_GetClockInputHz(CLKIN_MAINPLL);
	maxSpifiClock = spifiDevGetInfo(spifi, SPIFI_INFO_MAXCLOCK);

	Chip_Clock_SetDivider(CLK_IDIV_E, CLKIN_MAINPLL, ((spifiBaseClockRate / maxSpifiClock) + 1));

	spifiDevSetMemMode(spifi, 1);
}


static uint32_t x_page_size;
static uint32_t x_addr;
static uint32_t x_erased;
static uint32_t x_buf[1024];
static uint32_t x_seq;

/*
 * XMODEM block receive callback for writing flash
 */

static void on_xmodem_rx(uint8_t *buf, size_t len)
{
	uint32_t off_block = x_seq * 128;
	uint32_t off_page = (off_block / x_page_size) * x_page_size;

	if(x_erased == 0) {
		spifiDevSetMemMode(spifi, 0);
		spifiErase(spifi, 0, 1);
		x_erased = 1;
	}

	memcpy((void *)x_buf + (off_block % x_page_size), buf, len);

	if(((off_block + 128) % x_page_size) == 0) {
		spifiDevPageProgram(spifi, x_addr + off_page, x_buf, x_page_size);
	}

	x_seq ++;
}


void copy_ram_to_flash(void)
{
	uint32_t page_size = spifiDevGetInfo(spifi, SPIFI_INFO_PAGESIZE);
	uint32_t buf_aligned[page_size / 4];
	uint8_t *buf = (void *)&buf_aligned;
	extern uint8_t _srom, _sbss;
	uint8_t *from = &_srom;
	uint32_t size = (&_sbss - &_srom) + 16;

	/* Make first page with UDF header and first part of code */
	
	memset(buf_aligned, 0xff, sizeof(buf_aligned));
	buf[0] = 0xda;
	buf[1] = 0xff;
	buf[2] = ((size & 0x00ff) >>  0);
	buf[3] = ((size & 0xff00) >>  8);

	int skip = 16;
	uint32_t to = spifiGetAddrFromBlock(spifi, 0);

	/* Flash rom segments */
	
	printd("erase...");

	spifiDevSetMemMode(spifi, 0);
	spifiErase(spifi, 0, 1);
	
	printd("flash");

	while(from < &_sbss) {

		memcpy(buf+skip, from, page_size - skip);
		if((to % 4096) == 0) {
			printd(".");
		}
		SPIFI_ERR_T r = spifiDevPageProgram(spifi, to, buf_aligned, page_size);
		if(r != SPIFI_ERR_NONE) {
			printd("spifi err %d\n", r);
		}
		to += page_size;
		from += (page_size - skip);
		skip = 0;
	}

	printd("done\n");
}


static void *spifi_verify(void)
{
	extern uint8_t _stext, _edata, _srom;
	uint8_t *p1 = &_stext;
	uint8_t *p2 = (&_stext - &_srom) + (uint8_t *)0x80000010;
	uint32_t size = (&_edata - &_stext);
	uint32_t i;
	for(i=0; i<size; i++) {
		if(*p1 != *p2) {
			printd("Diff at %08x\n", p1);
			return p1;
			break;
		}
		p1++;
		p2++;
	}
	return NULL;
}



void spifi_check_update(void)
{
	void *pdiff = spifi_verify();
	if(pdiff != NULL) {
		printd("Software update detected\n");
		copy_ram_to_flash();
	}
}


static int on_cmd_flash(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc < 1) {
		return 0;
	}

	if(spifi == NULL) {
		cmd_printd(cli, "No flash detected\n");
		return 1;
	}

	int rv = 0;
	char cmd = argv[0][0];

	if(cmd == 'p') {
		copy_ram_to_flash();
		rv = 1;
	}

	if(cmd == 'v') {
		void *pdiff = spifi_verify();
		if(pdiff != NULL) {
			printd("Fail at %08x\n", pdiff);
		}
		rv = 1;
	}

	if(cmd == 'i') {

		uint16_t ver = spifiGetLibVersion();
		cmd_printd(cli, "SPIFI Lib Version   = %02d%02d\n", ((ver >> 8) & 0xff), (ver & 0xff));

		cmd_printd(cli, "Memory mode         = %d\n", spifiDevGetMemoryMode(spifi));
		cmd_printd(cli, "Device family       = %s\n", spifiDevGetFamilyName(spifi));
		cmd_printd(cli, "Capabilities        = 0x%x\n", spifiDevGetInfo(spifi, SPIFI_INFO_CAPS));
		cmd_printd(cli, "Device size         = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_DEVSIZE));
		cmd_printd(cli, "Erase blocks        = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_ERASE_BLOCKS));
		cmd_printd(cli, "Erase block size    = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_ERASE_BLOCKSIZE));
		cmd_printd(cli, "Erase sub-blocks    = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_ERASE_SUBBLOCKS));
		cmd_printd(cli, "Erase sub-blocksize = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_ERASE_SUBBLOCKSIZE));
		cmd_printd(cli, "Write page size     = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_PAGESIZE));
		cmd_printd(cli, "Max single readsize = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_MAXREADSIZE));
		cmd_printd(cli, "Current dev status  = 0x%x\n", spifiDevGetInfo(spifi, SPIFI_INFO_STATUS));
		cmd_printd(cli, "Current options     = %d\n", spifiDevGetInfo(spifi, SPIFI_INFO_OPTIONS));
		cmd_printd(cli, "SPIFI clock rate    = %d\n", Chip_Clock_GetClockInputHz(CLKIN_IDIVE));

		rv =  1;
	}

	if(cmd == 'e') {
		spifiErase(spifi, 0, 1);
	}

	if(cmd == 'x') {

		x_erased = 0;
		x_page_size = spifiDevGetInfo(spifi, SPIFI_INFO_PAGESIZE);
		x_seq = 0;
		x_addr = spifiGetAddrFromBlock(spifi, 0);

		//printd_enable(0);
		xmodem_receive(on_xmodem_rx, cli->tx, cli->rx);
		//printd_enable(1);

		rv = 1;
	}

	/* Restore memory mode */ 

	spifiDevSetMemMode(spifi, 1);

	return rv;
}


CMD_REGISTER(flash, on_cmd_flash, "i[nfo] | e[rase] | x[modem] | p[rogram]");

/*
 * End
 */

