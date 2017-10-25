
NAME	:= loader
BIN	:= $(NAME).bin

SRC     += boot.c
SRC     += main.c
SRC     += led.c

SRC	+= lpc_chip_43xx/src/chip_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/clock_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/sysinit_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/fpu_init.c

CLEAN	+= core.inc

# CPU specific flags

MFLAGS  = -mcpu=cortex-m4
MFLAGS += -mfloat-abi=softfp
MFLAGS += -mfpu=fpv4-sp-d16
MFLAGS += -ffast-math

CFLAGS	+= -DCORE_M4
CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx/inc/

all: $(BIN)

include ../Rules.mak

main.c: core.inc

core.inc: ../core/core.bin
	$(E) ../tools/bin2c $< > $@

-include $(DEPS)

