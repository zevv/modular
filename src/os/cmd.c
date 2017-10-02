

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "cmd.h"
#include "printd.h"
#include "uart.h"

#define CMD_MAX_PARTS 8


void cmd_init(void)
{
}


static int32_t split(char *buf, char *part[], int8_t maxparts)
{
	int8_t parts = 0;
	int32_t o2 = 0;

	if(buf[0] != '\0') {

		part[0] = buf;
		parts ++;

		for(;;) {
			while((buf[o2] != '\0') && (buf[o2] != ' ')) {
				o2 ++;
			}

			if(buf[o2] != '\0') {

				buf[o2] = '\0';
				o2 ++;

				while(buf[o2] == ' ') {
					o2 ++;
				}
			}
			
			if(buf[o2] == '\0') {
				break;
			}

			part[parts] = &buf[o2];
			parts ++;
		}
	}

	return parts;
}


static void handle_cmd(struct cmd_cli *cli, char *part[CMD_MAX_PARTS], int32_t parts)
{
	extern struct cmd_handler_t __start_cmd;
	extern struct cmd_handler_t __stop_cmd;

	struct cmd_handler_t *h = &__start_cmd;
	struct cmd_handler_t *h_found = NULL;

	int l = strlen(part[0]);

	while(h < &__stop_cmd) {
		if(h->name && strncmp(part[0], h->name, l) == 0) {
			if(h_found == NULL) {
				h_found = h;
			} else {
				cmd_printd(cli, "ambigious cmd\n");
				return;
			}
		}
		h++;
	}

	if(h_found) {
		int32_t argc = parts - 1;
		char **argv = &part[1];
		int handled = h_found->fn(cli, argc, argv);
		if(!handled) {
			cmd_printd(cli, "usage: %s %s\n", h_found->name, h_found->help);
		}
	} else {
		cmd_printd(cli, "unknown cmd\n");
	}
}


void cmd_cli_handle_char(struct cmd_cli *cli, uint8_t c)
{

	/* backspace */

	if((c == '\b') || (c == '\x7f')) {
		if(cli->len > 0) {
			cli->len--;
			cli->buf[cli->len] = '\0';
			cmd_printd(cli, "\b \b");
		}
	}

	/* text */

	if((c >= ' ') && (c < '\x7f')) {
		cli->buf[cli->len] = c;
		if(cli->len < (sizeof(cli->buf) - 1)) {
			cli->len++;
			cli->buf[cli->len] = '\0';
			cli->tx(c);
		}
	}

	/* newline */

	if((c == '\n') || (c == '\r')) {
		cli->tx('\n');
	
		char *part[CMD_MAX_PARTS];
		int32_t parts;

		parts = split(cli->buf, part, CMD_MAX_PARTS);

		if(parts > 0) {
			handle_cmd(cli, part, parts);
		
		}

		cli->len = 0;
		cli->buf[cli->len] = '\0';

		cmd_printd(cli, "\r> ");
	}
	
}


void cmd_cli_poll(struct cmd_cli *cli)
{
	int r;
	uint8_t c;

	r = cli->rx(&c);
	if(r) {
		cmd_cli_handle_char(cli, c);
	}
}


static int on_help(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	extern struct cmd_handler_t __start_cmd;
	extern struct cmd_handler_t __stop_cmd;

	struct cmd_handler_t *h = &__start_cmd;
	while(h < &__stop_cmd) {
		cmd_printd(cli, "%s: %s\n", h->name, h->help);
		h++;
	}

	return 1;
}


void cmd_printd(struct cmd_cli *cli, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintd(cli->tx, fmt, va);
	va_end(va);
}


void cmd_hexdump(struct cmd_cli *cli, void *addr, size_t len, off_t offset)
{
	fhexdump(cli->tx, addr, len, offset);
}


CMD_REGISTER(help, on_help, "");

/*
 * End
 */
