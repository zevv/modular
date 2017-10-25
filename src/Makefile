
SUBDIRS = tools core dsp modules loader

all: modular.dfu modules.img

include Rules.mak


modules: dsp

loader: core

tools/lpcsum:
	$(E) $(MAKE) -C tools lpcsum

tools/lpchdr:
	$(E) $(MAKE) -C tools lpchdr

tools/mkimg:
	$(E) $(MAKE) -C tools mkimg

modular.dfu: loader tools
	$(P) "LPCHDR $@"
	$(E) cp loader/loader.bin modular.dfu
	$(E) tools/lpchdr modular.dfu
	$(E) dfu-suffix --vid=0x1fc9 --pid=0x000c --did=0x0 -a modular.dfu

modules.img: tools modules
	$(P) "MODIMG"
	$(E) tools/mkimg modules/*/*bin > $@

usb: modular.dfu
	$(P) "INSTALL $<"
	$(E) chronic sudo dfu-util -d 1fc9:000c -a 0 -D modular.dfu

run: usb
	$(P) "RUN"
	$(E) while [ ! -c /dev/ttyACM0 ]; do sleep 0.1; /bin/echo -en "."; done
	$(E) sleep 0.1
	$(E) iterm /dev/ttyACM0
