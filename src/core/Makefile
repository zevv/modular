
NAME	:= core
BIN	:= $(NAME).bin
LIBS	+= -lm
CORE	:= m0

SRC     += adc.c
SRC     += can.c
SRC     += boot.c
SRC     += midi.c
SRC     += cmd.c
SRC     += flash.c
SRC     += i2s.c
SRC     += led.c
SRC     += button.c
SRC     += main.c
SRC     += mem.c
SRC     += printd.c
SRC     += ringbuffer.c
SRC     += uart.c
SRC     += ssm2604.c
SRC     += usbdesc.c
SRC     += watchdog.c
SRC     += mod.c
SRC     += mon.c
SRC     += ifft.c
SRC     += dpy.c

SRC	+= lpc_chip_43xx_m0/src/chip_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/adc_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/clock_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/i2c_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/i2s_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/ssp_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/sysinit_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/uart_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/wwdt_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/ccan_18xx_43xx.c

CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx_m0/inc/

include ../Rules.mak

-include $(DEPS)

