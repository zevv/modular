
NAME	:= core
BIN	:= $(NAME).bin

SRC     += adc.c
SRC     += boot.c
SRC     += cdc_uart.c
SRC     += cmd.c
SRC     += flash.c
SRC     += i2s.c
SRC     += led.c
SRC     += main.c
SRC     += mem.c
SRC     += printd.c
SRC     += ringbuffer.c
SRC     += uart.c
SRC     += uda1380.c
SRC     += usbdesc.c
SRC     += watchdog.c

SRC	+= lpc_chip_43xx_m0/src/chip_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/adc_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/clock_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/i2c_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/i2s_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/ssp_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/sysinit_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/uart_18xx_43xx.c
SRC	+= lpc_chip_43xx_m0/src/wwdt_18xx_43xx.c

# CPU specific flags

MFLAGS  = -mcpu=cortex-m0

CFLAGS	+= -DCORE_M0
CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx_m0/inc/

include ../Rules.mak

usb: $(DFU)
	$(P) " INSTALL $<"
	$(E) chronic sudo dfu-util -d 1fc9:000c -a 0 -D $(DFU)

.PHONY: tools clean

-include $(DEPS)

