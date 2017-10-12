#ifndef bios_cmd_h
#define bios_cmd_h

#include <unistd.h>
#include <stdbool.h>

struct cmd_cli {
	char buf[128];
	int len;
	void (*tx)(uint8_t c);
	int (*rx)(uint8_t *c);
	bool echo;
};


typedef int (*cmd_handler)(struct cmd_cli *cli, uint8_t argc, char **argv);

void cmd_init(struct cmd_cli *cli);
void cmd_cli_handle_char(struct cmd_cli *cli, uint8_t c);
void cmd_cli_poll(struct cmd_cli *cli);
void cmd_printd(struct cmd_cli *cli, const char *fmt, ...);
void cmd_hexdump(struct cmd_cli *cli, void *addr, size_t len, off_t offset);

struct cmd_handler_t {
	char const *name;
	cmd_handler fn;
	const char *help;
};

#define CMD_REGISTER(n, f, h) \
	static const struct cmd_handler_t cmd_handler_ ## n  \
	__attribute((section( "cmd" ), used )) = \
	{ .name = #n, .fn = f, .help = h }

#endif
