
TOP	:= ../..

NAME	:= osc
BIN	:= $(NAME).bin
LDS	:= ../../dsp/dsp.lds

SRC     += module.c
SRC     += osc.c
SRC     += sintab.c

LIBS	+= -lm ../../dsp/dsp.a

# CPU specific flags

MFLAGS  = -mcpu=cortex-m4
MFLAGS += -mthumb
MFLAGS += -mfloat-abi=softfp
MFLAGS += -mfpu=fpv4-sp-d16
MFLAGS += -ffast-math

CFLAGS	+= -DCORE_M4
CFLAGS	+= -I.
CFLAGS	+= -I../../dsp/lpc_chip_43xx/inc/

include ../../Rules.mak


