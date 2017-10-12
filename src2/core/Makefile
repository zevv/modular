
NAME	:= core
VERSION	:= 1.0
BUILD	:= 0

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
MFLAGS += -mthumb

# Standalone configuration, no startup code etc

CFLAGS  += -ffreestanding -nostartfiles $(MFLAGS)
LDFLAGS += -ffreestanding -nostartfiles $(MFLAGS)

# garbage-collect unused functions and data

CFLAGS	+= -ffunction-sections -fdata-sections 
LDFLAGS += -Wl,--gc-sections

CFLAGS	+= -DCORE_M0
CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx_m0/inc/

LDFLAGS += -Tcore.lds

include ../Rules.mak

../tools/lpcsum:
	$(E) $(MAKE) -C ../tools lpcsum

../tools/lpchdr:
	$(E) $(MAKE) -C ../tools lpchdr

$(ELF): $(OBJS) core.lds
	$(P) " LD $<"
	$(E) $(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(E) echo
	$(E) $(SIZE) $(ELF)
	$(E) echo

$(DFU): $(BIN) ../tools/lpchdr
	$(P) " LPCHDR $@"
	$(E) cp $(BIN) $(DFU)
	$(E) chronic dfu-suffix --vid=0x1fc9 --pid=0x000c --did=0x0 -a $(DFU)
	$(E) ../tools/lpchdr $(DFU)

usb: $(DFU)
	$(P) " INSTALL $<"
	$(E) chronic sudo dfu-util -d 1fc9:000c -a 0 -D $(DFU)

clean: 
	$(P) " CLEAN"
	$(E) rm -f $(DEPS) $(OBJS) $(ELF) $(BIN) $(DFU) *~ cscope.out cscope.files
	$(E) rm -f *.dfu *.bin
	$(E) $(MAKE) -C ../tools clean

.PHONY: tools clean

-include $(DEPS)

