
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "chip.h"
#include "cmd.h"
#include "flash.h"

#define CMD_READ_IDENTIFICATION             0x9F
#define CMD_READ_STATUS_REGISTER            0xD7
#define CMD_READ_DATA_BYTES                 0x03
#define CMD_ERASE_AND_WRITE                 0x83
#define CMD_MAIN_MEMORY_PAGE_PROGRAM        0x82

#define PAGE_SIZE 512


STATIC const PINMUX_GRP_T mux[] = {
	{0x3, 3,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},	/* SSP0 SCK */
	{0x3, 6,  (SCU_PINIO_FAST | SCU_MODE_FUNC0)},	/* SSP0 SSEL */
	{0x3, 7,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},	/* SSP0 MISO */
	{0x3, 8,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)}	/* SSP0 MOSI */
};

static void set_cs(int v)
{
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 6, v);
}


static void wait_busy(void)
{
	uint8_t cmd = CMD_READ_STATUS_REGISTER;
	uint8_t status[2];
	
	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, &cmd, sizeof(cmd));
	
	size_t n = 0;
	while(n < 15000) {
		Chip_SSP_ReadFrames_Blocking(LPC_SSP0, status, sizeof(status));
		if(status[0] & 0x80) {
			break;
		}
		n ++;
	}

	set_cs(1);
}


static void mk_cmd_addr(uint8_t cmd, uint32_t addr, uint8_t *buf, size_t len)
{
	uint32_t page = addr / 528;
	uint32_t off = addr % 528;

	uint32_t data = (cmd << 24) | (page << 10) | off;
	uint8_t *p = (void *)&data;
	int i;

	for(i=0; i<4; i++) {
		buf[i] = p[3-i];
	}
}


static void read_id(struct cmd_cli *cli)
{
	uint8_t cmd = CMD_READ_IDENTIFICATION;
	uint8_t buf[4] = { 0 };
	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, &cmd, sizeof(cmd));
	Chip_SSP_ReadFrames_Blocking(LPC_SSP0, buf, sizeof(buf));
	set_cs(1);
	cmd_printd(cli, "%02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
}


void flash_init(void)
{
	Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(PINMUX_GRP_T));

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 6);
	set_cs(1);
	
	Chip_SSP_Init(LPC_SSP0);
	Chip_SSP_SetBitRate(LPC_SSP0, 18000000);
        Chip_SSP_SetFormat(LPC_SSP0, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_CPHA0_CPOL0);
	Chip_SSP_Enable(LPC_SSP0);
}


void flash_erase(void)
{
	uint8_t cmd[4] = { 0xC7, 0x94, 0x80, 0x9A }; 

	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	set_cs(1);

	wait_busy();
}


void flash_read(uint32_t addr, void *buf, size_t len)
{
	uint8_t cmd[4];
	mk_cmd_addr(CMD_READ_DATA_BYTES, addr, cmd, sizeof(cmd));

	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	Chip_SSP_ReadFrames_Blocking(LPC_SSP0, buf, len);
	set_cs(1);
}



void flash_write(uint32_t addr, const void *buf, size_t len)
{
	uint8_t cmd[4];
	mk_cmd_addr(CMD_MAIN_MEMORY_PAGE_PROGRAM, addr, cmd, sizeof(cmd));

	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, buf, len);
	set_cs(1);

	wait_busy();
}


static void copy_ram_to_flash(void)
{
	
	extern uint8_t _srom, _sbss;
	uint8_t *from = &_srom;
	uint32_t size = (&_sbss - &_srom) + 16;
	
	/* UDF header */
	
	uint8_t buf[16];
	memset(buf, 0xff, sizeof(buf));
	buf[0] = 0xda;
	buf[1] = 0xff;
	buf[2] = ((size & 0x00ff) >>  0);
	buf[3] = ((size & 0xff00) >>  8);
	flash_write(0, buf, sizeof(buf));

	/* Copy firmare */

	uint32_t skip = 0x10;
	uint32_t to = 0x10;
	uint32_t blocksize = 528;

	while(from < &_sbss+blocksize) {
		flash_write(to, from, blocksize-skip);
		from += blocksize - skip;
		to += blocksize - skip;
		skip = 0;
	}
}


static int on_cmd_flash(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc < 1) {
		return 0;
	}

	char cmd = argv[0][0];

	if(cmd == 'i') {
		read_id(cli);
		return 1;
	}

	if(cmd == 'r' && argc > 1) {
		uint32_t addr = strtol(argv[1], NULL, 0);
		uint8_t buf[16];
		size_t i;
		for(i=0; i<256; i+=16) {
			flash_read(addr+i, buf, sizeof(buf));
			cmd_hexdump(cli, buf, sizeof(buf), addr+i);
		}
		return 1;
	}
	
	if(cmd == 'e') {
		flash_erase();
		return 1;
	}

	if(cmd == 'w') {
		flash_write(0x1234, "hier", 4);
		return 1;
	}

	if(cmd == 'c') {
		copy_ram_to_flash();
		return 1;
	}

	if(cmd == 't') {
		//flash_write(0x00, "een", 3);
		//flash_write(0x10, "twee", 3);

		uint8_t cmd[4] = { 0x03, 0, 0, 0 };
		uint8_t buf[16];
		size_t i;
		set_cs(0);
		Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
		for(i=0; i<1024; i+=16) {
			Chip_SSP_ReadFrames_Blocking(LPC_SSP0, buf, sizeof(buf));
			cmd_hexdump(cli, buf, sizeof(buf), i);
		}
		set_cs(1);
	}

	return 0;
}


CMD_REGISTER(flash, on_cmd_flash, "i[nfo] | r[ead] <addr>");

/*
 * ind
 */
