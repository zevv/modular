
TOP	:= ../..

BIN	:= $(NAME).bin
LDS	:= ../../dsp/dsp.lds
LIBS	+= ../../dsp/dsp.a
LIBS	+= -lm 
CFLAGS	+= -I../../dsp

$(BIN): $(LIBS)

# CPU specific flags

MFLAGS += -mfp16-format=alternative
MFLAGS += -mcpu=cortex-m4
MFLAGS += -mthumb
MFLAGS += -mfloat-abi=softfp
MFLAGS += -mfpu=fpv4-sp-d16
MFLAGS += -ffast-math

CFLAGS	+= -DCORE_M4
CFLAGS	+= -I.
CFLAGS	+= -I../../dsp/lpc_chip_43xx/inc/

PNG	:= $(NAME).png
SVG	:= $(NAME).svg

CLEAN	+= $(SVG) $(PNG)
#ALL	+= $(PNG)

include ../../Rules.mak

$(SVG): module.def
	$(P) " SVG $@"
	$(E) ../../tools/mod2svg $? > $@

$(PNG): $(SVG)
	$(P) " PNG $@"
	$(E) convert -density 300 -crop 312x312+0+0 $? $@




