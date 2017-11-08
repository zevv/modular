
SUBDIRS = tools core dsp modules loader

all: modular.dfu modules.img
	$(E) ls -lh modular.dfu
	$(E) ls -lh modules.img

include Rules.mak


modules: dsp

loader: core

tools/lpcsum:
	$(E) $(MAKE) -C tools lpcsum

tools/lpchdr:
	$(E) $(MAKE) -C tools lpchdr

tools/mkimg:
	$(E) $(MAKE) -C tools mkimg

tools/midimon:
	$(E) $(MAKE) -C tools midimon

.PHONY: tools/lpcsum tools/lpchdr tools/mkimg tools/midimon

modular.dfu: core loader tools
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

flash-loader: modular.dfu tools/midimon
	$(P) "FLASH"
	$(E) ./tools/flash modular.dfu 0 | tools/midimon

flash-modules: modules.img tools/midimon
	$(P) "FLASH"
	$(E) ./tools/flash modules.img 0x21000 | tools/midimon

flash: flash-loader flash-modules
