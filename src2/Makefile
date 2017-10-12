
all: core loader dsp
.PHONY: core loader dsp

core:
	make -C core bin

loader:
	make -C loader dfu

dsp:
	make -C dsp bin

clean:
	make -C core clean
	make -C loader clean
	make -C dsp clean
