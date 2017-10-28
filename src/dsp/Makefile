
NAME	:= dsp
LIB	:= $(NAME).a
CORE	:= m4

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

CFLAGS	+= -I.
CFLAGS	+= -Ilpc_chip_43xx/inc/

include ../Rules.mak


