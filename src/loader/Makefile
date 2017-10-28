
NAME	:= loader
BIN	:= $(NAME).bin
CORE	:= m4

SRC     += boot.c
SRC     += main.c
SRC     += led.c

SRC	+= lpc_chip_43xx/src/chip_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/clock_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/sysinit_18xx_43xx.c
SRC	+= lpc_chip_43xx/src/fpu_init.c

CLEAN	+= core.inc

CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx/inc/

all: $(BIN)

include ../Rules.mak

main.c: core.inc

core.inc: ../core/core.bin
	$(E) ../tools/bin2c $< > $@

-include $(DEPS)

