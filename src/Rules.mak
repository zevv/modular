
VERSION	:= 1.0

ELF	?= $(NAME).elf
LDS	?= $(NAME).lds

CFLAGS	+= -ggdb -O3
CFLAGS	+= -Wall -Werror -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-strict-overflow

CFLAGS	+= -DSRATE=44270.83
CFLAGS	+= -DVERSION=\"$(VERSION)\"
CFLAGS	+= -DBUILD=\"$(shell git rev-parse --short HEAD)\"
CFLAGS  += -fsingle-precision-constant

MFLAGS	 = -ffast-math
MFLAGS	+= -mthumb

# Standalone configuration, no startup code etc

CFLAGS  += -ffreestanding -nostartfiles $(MFLAGS)
LDFLAGS += -ffreestanding -nostartfiles $(MFLAGS)

# garbage-collect unused functions and data

CFLAGS	+= -ffunction-sections -fdata-sections 
LDFLAGS += -Wl,--gc-sections
	
ALL	+= $(SUBDIRS) $(LIB) $(BIN)

ifeq ($(CORE), m0)
  MFLAGS  += -mcpu=cortex-m0
  CFLAGS  += -DCORE_M0
  CFLAGS  += -I.
  CFLAGS  += -Ilpc_chip_43xx_m0/inc/
endif

ifeq ($(CORE), m4)
  MFLAGS  += -mcpu=cortex-m4
  MFLAGS  += -mfloat-abi=softfp
  MFLAGS  += -mfpu=fpv4-sp-d16
  MFLAGS  += -mfp16-format=alternative
  MFLAGS  += -ffast-math
  CFLAGS  += -DCORE_M4 -DARM_MATH_CM4
  CFLAGS  += -I.
  CFLAGS  += -Ilpc_chip_43xx/inc/
endif

CROSS	:= /opt/toolchains/gcc-arm-none-eabi-5_4-2016q3/bin/arm-none-eabi-
AS      := $(CROSS)as
LD      := $(CROSS)ld
CC      := $(CROSS)gcc
AR      := $(CROSS)ar
NM      := $(CROSS)nm
STRIP   := $(CROSS)strip
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
SIZE    := $(CROSS)size
CCACHE	:= ccache

OBJS    = $(subst .c,.o, $(SRC))
DEPS    = $(subst .c,.d, $(SRC))

CLEAN	+= $(BIN) $(ELF) $(LIB) $(OBJS) $(DEPS)
CLEAN	+= cscope.*

ifdef V
E       =
P       = @true
MAKE	= make
else
E       = @
P       = @echo
MAKE    = make -s
endif

.PHONY: $(SUBDIRS)
.PRECIOUS: $(BIN) $(ELF)

all: $(ALL)
bin: $(BIN)
lib: $(LIB)

clean:
	$(P) " CLEAN"
	$(E) rm -f $(CLEAN)
	$(E) for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done
	
%.o: %.c
	$(P) " CC $<"
	$(E) $(CCACHE) $(CC) $(CFLAGS) -MMD -c $< -o $@

$(SUBDIRS):
	$(P) "SUB $@"
	$(E) $(MAKE) -C $@

$(ELF): $(OBJS) $(LDS)
	$(P) " LD $<"
	$(E) $(CC) -T$(LDS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BIN): $(ELF) 
	$(P) " BIN $@"
	$(E) $(OBJCOPY) -O binary $(ELF) $(BIN)

$(LIB): $(OBJS)
	$(P) " LIB $@"
	$(E) chronic $(AR) $(ARFLAGS) $@ $?

