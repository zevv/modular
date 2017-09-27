#!/bin/sh

/opt/toolchains/arm-2014.05/bin/arm-none-eabi-gcc \
	-c mul.c \
	-DCORE_M4 \
	-D__USE_LPCOPEN \
	-mcpu=cortex-m4 \
	-mthumb \
	-mfloat-abi=softfp \
	-mfpu=fpv4-sp-d16 \
	-g

/opt/toolchains/arm-2012.03/bin/arm-none-eabi-objdump -D mul.o
