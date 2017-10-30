
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "chip.h"
#include "cmd.h"
#include "watchdog.h"
#include "printd.h"
#include "flash.h"
#include "uart.h"

#define CMD_READ_IDENTIFICATION             0x9F
#define CMD_READ_STATUS_REGISTER            0xD7
#define CMD_READ_DATA_BYTES                 0x03
#define CMD_ERASE_AND_WRITE                 0x83
#define CMD_MAIN_MEMORY_PAGE_PROGRAM        0x82
#define CMD_WRITE_BUFFER_1                  0x84
#define CMD_PAGE_ERASE                      0x81

#define PAGE_SIZE 528


STATIC const PINMUX_GRP_T mux[] = {
	{0x3, 3,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},	/* SSP0 SCK */
	{0x3, 6,  (SCU_PINIO_FAST | SCU_MODE_FUNC0)},	/* SSP0 SSEL */
	{0x3, 7,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},	/* SSP0 MISO */
	{0x3, 8,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)}	/* SSP0 MOSI */
};

static void set_cs(int v)
{
	volatile int i;
	for(i=0; i<200; i++);
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 6, v);
	for(i=0; i<1000; i++);
}


static void wait_busy(bool do_watchdog)
{
	uint8_t cmd = CMD_READ_STATUS_REGISTER;
	uint8_t status[2];
	
	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, &cmd, sizeof(cmd));
	
	for(;;) {
		Chip_SSP_ReadFrames_Blocking(LPC_SSP0, status, sizeof(status));
		if(status[0] & 0x80) {
			break;
		}
		if(do_watchdog) {
			watchdog_poll();
		}
	}

	set_cs(1);
}


static void mk_cmd_addr(uint8_t cmd, uint32_t addr, uint8_t *buf, size_t len)
{
	uint32_t page = addr / PAGE_SIZE;
	uint32_t off = addr % PAGE_SIZE;

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


void flash_page_erase(uint32_t addr)
{
	uint8_t cmd[4];
	mk_cmd_addr(CMD_PAGE_ERASE, addr, cmd, sizeof(cmd));

	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	set_cs(1);

	wait_busy(true);
}


void flash_erase(void)
{
	uint8_t cmd[4] = { 0xC7, 0x94, 0x80, 0x9A }; 

	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	set_cs(1);

	wait_busy(true);
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


/*
 * Load data in flash buffer 1
 */

void flash_load(uint32_t addr, const void *buf, size_t len)
{
	uint8_t cmd[4];
	mk_cmd_addr(CMD_WRITE_BUFFER_1, addr, cmd, sizeof(cmd));

	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, buf, len);
	set_cs(1);

	wait_busy(false);
}

/*
 * Erase page and write buffer 1
 */

void flash_write_buf1(uint32_t addr)
{
	uint8_t cmd[4];
	mk_cmd_addr(CMD_ERASE_AND_WRITE, addr, cmd, sizeof(cmd));
	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	set_cs(1);
	wait_busy(false);
}


void flash_write(uint32_t addr, const void *buf, size_t len)
{
	uint8_t cmd[4];
	mk_cmd_addr(CMD_MAIN_MEMORY_PAGE_PROGRAM, addr, cmd, sizeof(cmd));

	set_cs(0);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, cmd, sizeof(cmd));
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, buf, len);
	set_cs(1);

	wait_busy(false);
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
	
	if(cmd == 'l' && argc > 3) {
		uint32_t addr = strtol(argv[1], NULL, 16);
		uint8_t sum1 = strtol(argv[2], NULL, 16);
		uint8_t sum2 = 0;
		uint8_t buf[argc-3];
		size_t i;
		for(i=3; i<argc; i++) {
			uint8_t c = strtol(argv[i], NULL, 16);
			sum2 += c;
			buf[i-3] = c;
		}
		if(sum1 == sum2) {
			flash_load(addr, buf, sizeof(buf));
		} else {
			cmd_printd(cli, "%06x crc\n", addr);
		}
		return 1;
	}

	if(cmd == 'W' && argc > 1) {
		cmd_printd(cli, ".");
		uint32_t addr = strtol(argv[1], NULL, 16);
		flash_write_buf1(addr);
		return 1;
	}

	if(cmd == 'w' && argc > 3) {
		uint32_t addr = strtol(argv[1], NULL, 16);

		if((addr % PAGE_SIZE) == 0) {
			cmd_printd(cli, ".");
		}

		uint8_t sum1 = strtol(argv[2], NULL, 16);
		uint8_t sum2 = 0;
		uint8_t buf[argc-3];
		uint8_t buf2[argc-3];
		size_t i;
		for(i=3; i<argc; i++) {
			uint8_t c = strtol(argv[i], NULL, 16);
			sum2 += c;
			buf[i-3] = c;
		}
		if(sum1 == sum2) {
			flash_write(addr, buf, sizeof(buf));
			flash_read(addr, buf2, sizeof(buf2));
			if(memcmp(buf, buf2, sizeof(buf)) != 0) {
				cmd_printd(cli, "%06x vrfy\n", addr);
				cmd_hexdump(cli, buf, sizeof(buf), i);
				cmd_hexdump(cli, buf2, sizeof(buf2), i);
			}
		} else {
			cmd_printd(cli, "%06x crc\n", addr);
		}
		return 1;
	}

	if(cmd == 's') {

		/* Read length from DFU header */

		uint16_t len;
		uint32_t sum1;
		flash_read(2, (void *)&len, sizeof(len));
		flash_read(4, (void *)&sum1, sizeof(sum1));

		uint32_t addr = 16;
		uint32_t sum2 = 0;

		while(len > 0) {
			uint8_t buf[16];
			size_t l = (len > sizeof(buf)) ? sizeof(buf) : len;
			flash_read(addr, buf, l);
			size_t i;
			for(i=0; i<l; i++) {
				sum2 = sum2 + buf[i] + 1;
			}
			addr += l;
			len -= l;
			watchdog_poll();
		}
		cmd_printd(cli, sum1 == sum2 ? "ok:" : "ERR:");
		cmd_printd(cli, " sum=%08x sum2=%08x\n", sum1, sum2);
		return 1;
	}

	return 0;
}


CMD_REGISTER(flash, on_cmd_flash, "i[nfo] | r[ead] <addr>");

/*
 * ind
 */
