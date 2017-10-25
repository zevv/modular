
SUBDIRS = tools core dsp modules loader

all: modular.dfu

include Rules.mak


modules: dsp

loader: core

tools/lpcsum:
	$(E) $(MAKE) -C tools lpcsum

tools/lpchdr:
	$(E) $(MAKE) -C tools lpchdr

modular.dfu: loader tools/lpcsum tools/lpchdr
	$(P) " LPCHDR $@"
	$(E) cp loader/loader.bin modular.dfu
	$(E) chronic dfu-suffix --vid=0x1fc9 --pid=0x000c --did=0x0 -a modular.dfu
	$(E) tools/lpchdr modular.dfu

usb: modular.dfu
	$(P) " INSTALL $<"
	$(E) chronic sudo dfu-util -d 1fc9:000c -a 0 -D modular.dfu


