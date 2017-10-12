ELF	:= $(NAME).elf
BIN	:= $(NAME)-$(VERSION)-$(BUILD).bin
DFU	:= $(NAME)-$(VERSION)-$(BUILD).dfu

CFLAGS	+= -ggdb -O3
CFLAGS	+= -Wall -Werror

CFLAGS	+= -DNAME=\"$(NAME)\"
CFLAGS	+= -DVERSION=\"$(VERSION)\"
CFLAGS	+= -DBUILD=\"$(BUILD)\"

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
MAKE	:= make -s

OBJS    = $(subst .c,.o, $(SRC))
DEPS    = $(subst .c,.d, $(SRC))

E	= @
P	= @echo

all: $(ELF)
bin: $(BIN)
dfu: $(DFU)

%.o: %.c
	$(P) " CC $<"
	$(E) $(CCACHE) $(CC) $(CFLAGS) -MMD -c $< -o $@

$(BIN): $(ELF) ../tools/lpcsum
	$(P) " LPCSUM $@"
	$(E) $(OBJCOPY) -O binary -R pkt $(ELF) $(BIN)
	$(E) ../tools/lpcsum $(BIN)

