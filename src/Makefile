
NAME	:= modular
VERSION	:= 1.0
BUILD	:= 0
TTY	?= /dev/ttyUSB0

SRC     += main.c
SRC     += module.c
SRC     += panel.c

SRC     += $(wildcard modules/*.c)

SRC     += lib/biquad.c
SRC     += lib/osc.c
SRC     += lib/sintab.c
SRC     += lib/noise.c
SRC     += os/adc.c
SRC     += os/i2s.c
SRC     += os/board.c
SRC     += os/board_sysinit.c
SRC     += os/printd.c
SRC     += os/startup.c
SRC     += os/fault.c
SRC     += os/ringbuffer.c
SRC     += os/sysinit.c
SRC     += os/syscalls.c
SRC     += os/uart.c
SRC     += os/spifi.c
SRC     += os/cmd.c
SRC     += os/cdc_uart.c
SRC     += os/xmodem.c
SRC     += os/cdc_desc.c
SRC     += os/mem.c
SRC	+= lpcopen/lpc_chip_43xx/src/adc_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/i2s_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/gpdma_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/uart_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/gpio_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/clock_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/ring_buffer.c
SRC     += lpcopen/lpc_chip_43xx/src/fpu_init.c
SRC     += lpcopen/lpc_chip_43xx/src/chip_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/i2c_18xx_43xx.c
SRC     += lpcopen/lpc_chip_43xx/src/sysinit_18xx_43xx.c
SRC     += lpcopen/lpc_board_ngx_xplorer_4330/src/uda1380.c

LIBS	+= -lm
LIBS	+= ./lpcopen/lib_lpcspifilib/lib/liblpcspifilib_M3.a

ifdef CDC_UART
SRC	+= cdc_uart.c cdc_desc.c
CFLAGS	+= -DCDC_UART
endif

# CPU specific flags

MFLAGS  = -mcpu=cortex-m4
MFLAGS += -mthumb
MFLAGS += -mfloat-abi=softfp
MFLAGS += -mfpu=fpv4-sp-d16
MFLAGS += -ffast-math

# Standalone configuration, no startup code etc

CFLAGS  += -ffreestanding -nostartfiles $(MFLAGS)
LDFLAGS += -ffreestanding -nostartfiles $(MFLAGS)

# garbage-collect unused functions and data

#CFLAGS	+= -ffunction-sections -fdata-sections 
#LDFLAGS += -Wl,--gc-sections

# Always prefer float over doubles (M4 does not support double in hw)

CFLAGS  += -Wdouble-promotion -fsingle-precision-constant

CFLAGS	+= -ggdb -Os
CFLAGS	+= -Wall -Werror

CFLAGS	+= -DNAME=\"$(NAME)\"
CFLAGS	+= -DVERSION=\"$(VERSION)\"
CFLAGS	+= -DBUILD=\"$(BUILD)\"

CFLAGS	+= -DCORE_M4 -D__USE_LPCOPEN 
CFLAGS	+= -I.
CFLAGS	+= -I./os
CFLAGS	+= -I./lpcopen/lpc_chip_43xx/inc/
CFLAGS	+= -I./lpcopen/lib_lpcspifilib/inc
CFLAGS	+= -I./lpcopen/lpc_board_ngx_xplorer_4330/inc

LDFLAGS += -Trom.lds

CROSS 	:= /opt/toolchains/arm-2014.05/bin/arm-none-eabi-

AS      := $(CROSS)as
LD      := $(CROSS)ld
CC      := $(CROSS)gcc
AR      := $(CROSS)ar
NM      := $(CROSS)nm
STRIP   := $(CROSS)strip
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
SIZE    := $(CROSS)size
CCACHE	:= ccache
MAKE	:= make -s

OBJS    = $(subst .c,.o, $(SRC))
DEPS    = $(subst .c,.d, $(SRC))

E	= @
P	= @echo

ELF = $(NAME).elf
BIN = $(NAME)-$(VERSION)-$(BUILD).bin
DFU = $(NAME)-$(VERSION)-$(BUILD).dfu

all: $(DFU)

tools/lpcsum:
	$(E) $(MAKE) -C tools lpcsum

tools/lpchdr:
	$(E) $(MAKE) -C tools lpchdr

.c.o:
	$(P) " CC $<"
	$(E) $(CCACHE) $(CC) $(CFLAGS) -MMD -c $< -o $@

$(ELF): $(OBJS) rom.lds
	$(P) " LD $<"
	$(E) $(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(E) echo
	$(E) $(SIZE) $(ELF)
	$(E) echo

$(BIN): $(ELF) tools/lpcsum
	$(P) " LPCSUM $@"
	$(E) $(OBJCOPY) -O binary -R pkt $(ELF) $(BIN)
	$(E) ./tools/lpcsum $(BIN)

$(DFU): $(BIN) tools/lpchdr
	$(P) " LPCHDR $@"
	$(E) cp $(BIN) $(DFU)
	$(E) chronic dfu-suffix --vid=0x1fc9 --pid=0x000c --did=0x0 -a $(DFU)
	$(E) ./tools/lpchdr $(DFU)

usb: $(DFU)
	$(P) " INSTALL $<"
	$(E) chronic sudo dfu-util -d 1fc9:000c -a 0 -D $(DFU)

flash: $(DFU)
	$(P) " FLASH $<"
	$(E) -fuser -k $(TTY)
	$(E) echo "" > $(TTY)
	$(E) echo "flash x" > $(TTY)
	$(E) sx $(DFU) < $(TTY) > $(TTY)

clean: 
	$(P) " CLEAN"
	$(E) rm -f $(DEPS) $(OBJS) $(ELF) $(BIN) $(DFU) *~ cscope.out cscope.files
	$(E) rm -f *.dfu *.bin
	$(E) $(MAKE) -C tools clean

.PHONY: tools clean

-include $(DEPS)

