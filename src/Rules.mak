TOP	?= ../
ELF	?= $(NAME).elf
LDS	?= $(NAME).lds
DFU	?= $(NAME).dfu

CFLAGS	+= -ggdb -O3
CFLAGS	+= -Wall -Werror

CFLAGS	+= -DSRATE=48000
CFLAGS	+= -DNAME=\"$(NAME)\"
CFLAGS	+= -DVERSION=\"$(VERSION)\"
CFLAGS	+= -DBUILD=\"$(BUILD)\"
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

all: $(ALL)
	echo "ALL was $(ALL)"

bin: $(BIN)
lib: $(LIB)
dfu: $(DFU)

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

$(BIN): $(ELF) $(TOP)/tools/lpcsum
	$(P) " LPCSUM $@"
	$(E) $(OBJCOPY) -O binary -R pkt $(ELF) $(BIN)
	$(E) $(TOP)/tools/lpcsum $(BIN)

$(LIB): $(OBJS)
	$(P) " LIB $@"
	$(E) chronic $(AR) $(ARFLAGS) $@ $?

$(DFU): $(BIN) ../tools/lpchdr
	$(P) " LPCHDR $@"
	$(E) cp $(BIN) $(DFU)
	$(E) chronic dfu-suffix --vid=0x1fc9 --pid=0x000c --did=0x0 -a $(DFU)
	$(E) ../tools/lpchdr $(DFU)

