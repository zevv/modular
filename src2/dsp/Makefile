
NAME	:= dsp
VERSION	:= 1.0
BUILD	:= 0

SRC     += boot.c
SRC     += reverb/main.c
SRC     += reverb/reverb.c
SRC     += reverb/osc.c
SRC     += reverb/sintab.c
SRC	+= fpu_init.c

LIBS	+= -lm

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

CFLAGS	+= -DCORE_M4
CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx/inc/

LDFLAGS += -Tdsp.lds


ELF = $(NAME).elf
BIN = $(NAME)-$(VERSION)-$(BUILD).bin
DFU = $(NAME)-$(VERSION)-$(BUILD).dfu

all: $(BIN)

include ../Rules.mak

$(ELF): $(OBJS) dsp.lds
	$(P) " LD $<"
	$(E) $(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(E) echo
	$(E) $(SIZE) $(ELF)
	$(E) echo

.PHONY: clean

-include $(DEPS)

