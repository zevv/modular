
NAME	:= dsp
LIB	:= $(NAME).a

SRC	+= main.c
SRC     += boot.c
SRC	+= fpu_init.c
SRC	+= osc.c
SRC	+= biquad.c
SRC	+= sintab.c
SRC	+= adsr.c
SRC	+= fir.c
SRC	+= pot.c

LIBS	+= -lm

# CPU specific flags

MFLAGS  = -mcpu=cortex-m4
MFLAGS += -mthumb
MFLAGS += -mfloat-abi=softfp
MFLAGS += -mfpu=fpv4-sp-d16

CFLAGS	+= -DCORE_M4
CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx/inc/

include ../Rules.mak


