#include <stdint.h>
#include "chip.h"
#include "printd.h"
#include "cmd.h"

#include "ssm2604.h"

#define I2C_ADDR (0x34 >> 1)

#define REG_LEFT_CHANNEL_ADC_INPUT_VOLUME 0x00
#define  LRINBOTH (1<<8)
#define  LINMUTE (1<<7)
#define  LINVOL(v) (v & 0x3f)
#define REG_RIGHT_CHANNEL_ADC_INPUT_VOLUME 0x01
#define  RLINBOTH (1<<8)
#define  RINMUTE (1<<7)
#define  RINVOL(v) (v & 0x3f)
#define REG_ANALOG_AUDIO_PATH 0x04
#define  DACSEL (1<<4)
#define  BYBASS (1<<3)
#define REG_DIGITAL_AUDIO_PATH 0x05
#define  HPOR (1<<4)
#define  DACMU (1<<3)
#define  DEEMPH(v) ((v & 0x03) << 1)
#define  ADCHPF (1<<0)
#define REG_POWER_MANAGEMENT 0x06
#define  PWROFF (1<<7)
#define  CLKOUT (1<<6)
#define  OSC (1<<5)
#define  OUT (1<<4)
#define  DAC (1<<3)
#define  ADC (1<<2)
#define  LINEIN (1<<0)
#define REG_DIGITAL_AUDIO_I_F 0x07
#define  BCLKINV (1<<7)
#define  MS (1<<6)
#define  LRSWAP (1<<5)
#define  LRP (1<<4)
#define  WL(v) ((v & 0x03) << 2))
#define  FORMAT(v) ((v & 0x03) << 0)
#define REG_SAMPLING_RATE 0x08
#define  CLKODIV2 (1<<7)
#define  CLKDIV2 (1<<6)
#define  SR(v) ((v & 0x0f) << 2)
#define  BOSR (1<<1)
#define  USB (1<<0)
#define REG_ACTIVE 0x09
#define  ACTIVE (1<<0)
#define REG_SOFTWARE_RESET 0x0F
#define  RESET(v) (v)



static void reg_write(uint8_t reg, uint16_t val)
{
	uint8_t buf[2] = { 
		(reg << 1) | ((val & 0x100) >> 8), 
		val & 0xff,
	};
	printd("%02x %02x\n", buf[0], buf[1]);
	Chip_I2C_MasterSend(I2C0, I2C_ADDR, buf, sizeof(buf));
}


static uint16_t reg_read(uint8_t reg)
{
	uint8_t c[2];
	Chip_I2C_MasterCmdRead(I2C0, I2C_ADDR, reg << 1, c, sizeof(c));
	printd("%02x %02x\n", c[1], c[0]);
	return c[0] | (c[1] << 8);
}

/*
 * CONTROL REGISTER SEQUENCING
 * 1. Enable all of the necessary power management bits of
 *      Register R6 with the exception of the out bit (Bit D4). The
 *      out bit should be set to 1 until the final step of the control
 *      register sequence.
 * 2. After the power management bits are set, program all other
 *      necessary registers, with the exception of the active bit
 *      [Register R9, Bit D0] and the out bit of the power
 *      management register.
 * 3. As described in the Digital Core section of the Theory of
 *      Operation, insert enough delay time to charge the VMID
 *      decoupling capacitor before setting the active bit [Register
 *      R9, Bit D0].
 * 4. Finally, to enable the DAC output path of the SSM2603, set
 *      the out bit of Register R6 to 0.
 */

void ssm2604_init(void)
{
	Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);
	Chip_I2C_Init(I2C0);
	Chip_I2C_SetClockRate(I2C0, 100000);
	Chip_I2C_SetMasterEventHandler(I2C0, Chip_I2C_EventHandlerPolling);

	reg_write(REG_SOFTWARE_RESET, RESET(1));
	reg_write(REG_ACTIVE, 0);
	reg_write(REG_POWER_MANAGEMENT, OUT);
	reg_write(REG_LEFT_CHANNEL_ADC_INPUT_VOLUME, LINVOL(0x17));
	reg_write(REG_RIGHT_CHANNEL_ADC_INPUT_VOLUME, RINVOL(0x17));
	reg_write(REG_ANALOG_AUDIO_PATH, DACSEL);
	reg_write(REG_DIGITAL_AUDIO_PATH, 0);
	reg_write(REG_SAMPLING_RATE, BOSR);
	reg_write(REG_DIGITAL_AUDIO_I_F, FORMAT(2));
	reg_write(REG_ACTIVE, ACTIVE);

}



static int on_cmd_ssm(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc >= 1u) {
		char cmd = argv[0][0];

		if(cmd == 'i') {
			ssm2604_init();
			return 1;
		}
		
		if(cmd == 'r') {
			int reg = strtol(argv[1], NULL, 0);
			cmd_printd(cli,"%03x\n", reg_read(reg));
			return 1;
		}

		if(cmd == 'w') {
			int reg = strtol(argv[1], NULL, 0);
			int val = strtol(argv[2], NULL, 0);
			reg_write(reg, val);
			return 1;
		}

		if(cmd == 'd') {
			int i;
			for(i=0; i<10; i++) {
				cmd_printd(cli, "%0d %03x\n", i, reg_read(i));
			}
			return 1;
		}

	}
	return 0;
}

CMD_REGISTER(ssm, on_cmd_ssm, "");

/*
 * End
 */

