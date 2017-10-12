
NAME	:= loader
VERSION	:= 1.0
BUILD	:= 0

SRC     += boot.c
SRC     += main.c
SRC     += led.c

SRC	+= lpc_chip_43xx/src/chip_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/clock_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/sysinit_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/fpu_init.c

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

CFLAGS	+= -ffunction-sections -fdata-sections 
LDFLAGS += -Wl,--gc-sections

# Always prefer float over doubles (M4 does not support double in hw)

CFLAGS  += -Wdouble-promotion -fsingle-precision-constant

CFLAGS	+= -ggdb -O3
CFLAGS	+= -Wall -Werror

CFLAGS	+= -DNAME=\"$(NAME)\"
CFLAGS	+= -DVERSION=\"$(VERSION)\"
CFLAGS	+= -DBUILD=\"$(BUILD)\"

CFLAGS	+= -DCORE_M4
CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx/inc/

LDFLAGS += -Tloader.lds


ELF = $(NAME).elf
BIN = $(NAME)-$(VERSION)-$(BUILD).bin
DFU = $(NAME)-$(VERSION)-$(BUILD).dfu

all: $(BIN)

include ../Rules.mak

main.c: core.inc
core.inc: ../core/core-$(VERSION)-$(BUILD).bin
	$(E) ../tools/bin2c $< > $@

../tools/lpcsum:
	$(E) $(MAKE) -C ../tools lpcsum

../tools/lpchdr:
	$(E) $(MAKE) -C ../tools lpchdr

$(ELF): $(OBJS) loader.lds
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
	$(E) rm -f core.inc
	$(E) $(MAKE) -C ../tools clean

.PHONY: tools clean

-include $(DEPS)

