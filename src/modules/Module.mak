
TOP	:= ../..

BIN	:= $(NAME).bin
LDS	:= ../../dsp/dsp.lds
LIBS	+= ../../dsp/dsp.a
LIBS	+= ../../dsp/libarm_cortexM4l_math.a
LIBS	+= -lm 
CFLAGS	+= -I../../dsp
CORE	:= m4

$(BIN): $(LIBS)

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

png: $(PNG)



