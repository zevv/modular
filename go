#!/bin/sh

/opt/toolchains/arm-2012.03/bin/arm-none-eabi-gcc \
	-Trom.lds \
	lpc_chip_43xx/src/i2s_18xx_43xx.c \
	lpc_chip_43xx/src/gpdma_18xx_43xx.c \
	lpc_chip_43xx/src/uart_18xx_43xx.c \
	lpc_chip_43xx/src/gpio_18xx_43xx.c \
	lpc_chip_43xx/src/clock_18xx_43xx.c \
	lpc_chip_43xx/src/ring_buffer.c \
	lpc_chip_43xx/src/fpu_init.c \
	lpc_chip_43xx/src/chip_18xx_43xx.c \
	lpc_chip_43xx/src/i2c_18xx_43xx.c \
	lpc_chip_43xx/src/sysinit_18xx_43xx.c \
	periph_i2s/example/src/*.c \
	lpc_board_ngx_xplorer_4330/src/board.c \
	lpc_board_ngx_xplorer_4330/src/board_sysinit.c \
	lpc_board_ngx_xplorer_4330/src/uda1380.c \
	-I./lpc_chip_43xx/inc/ \
	-I./lib_lpcspifilib/inc \
	-I./lpc_board_ngx_xplorer_4330/inc \
	-DCORE_M4 \
	-D__USE_LPCOPEN \
	-mcpu=cortex-m4 \
	-mthumb \
	-mfloat-abi=soft \
	-mfpu=fpv4-sp-d16 

