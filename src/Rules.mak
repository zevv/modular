
VERSION	:= 1.0

ELF	?= $(NAME).elf
LDS	?= $(NAME).lds

CFLAGS	+= -ggdb -Os
CFLAGS	+= -Wall -Werror

CFLAGS	+= -DSRATE=44270.83
CFLAGS	+= -DVERSION=\"$(VERSION)\"
CFLAGS	+= -DBUILD=\"$(shell git rev-parse --short HEAD)\"
CFLAGS  += -Wdouble-promotion -fsingle-precision-constant

MFLAGS	+= -ffast-math

MFLAGS	+= -mthumb

# Standalone configuration, no startup code etc

CFLAGS  += -ffreestanding -nostartfiles $(MFLAGS)
LDFLAGS += -ffreestanding -nostartfiles $(MFLAGS)

# garbage-collect unused functions and data

CFLAGS	+= -ffunction-sections -fdata-sections 
LDFLAGS += -Wl,--gc-sections
	
ALL	+= $(SUBDIRS) $(LIB) $(BIN)


CROSS 	:= /opt/toolchains/arm-2014.05/bin/arm-none-eabi-

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
	$(E) rm -f $(BIN) $(ELF) $(LIB) $(OBJS) $(DEPS) $(CLEAN)
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
	$(E) $(OBJCOPY) -O binary -R pkt $(ELF) $(BIN)

$(LIB): $(OBJS)
	$(P) " LIB $@"
	$(E) chronic $(AR) $(ARFLAGS) $@ $?

