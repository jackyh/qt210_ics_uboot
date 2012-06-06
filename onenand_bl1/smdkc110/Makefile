#
# BL1 of U-Boot for OneNAND
#

sinclude ../../include/autoconf.mk

CROSS_COMPILE	= /usr/local/arm/4.2.2-eabi/usr/bin/arm-linux-

AS				= $(CROSS_COMPILE)as
LD				= $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump

MKBL1			= mkbl1

TARGET          = BL1.bin
O_TARGET		= BL1.elf
OBJS			= Init.o
OBJS            += Main.o

ifdef CONFIG_EVT1
FLAG4REV		= S5PC110_EVT1
TEXT_BASE       = 0xD0020010
BL1_LDS         = BL1_evt1.lds
else
FLAG4REV		= S5PC110_EVT0
TEXT_BASE       = 0xD0030000
BL1_LDS         = BL1.lds
endif

ifdef CONFIG_MCP_SINGLE
AFLAGS			= --defsym $(FLAG4REV)=0x1 --defsym S5PV210=0x1
CFLAGS			= -I../../include -Os -D$(FLAG4REV) -DS5PV210
CFLAGS_HOST		= -D$(FLAG4REV) -DS5PV210
else
AFLAGS			= --defsym $(FLAG4REV)=0x1
CFLAGS			= -I../../include -Os -D$(FLAG4REV)
CFLAGS_HOST		= -D$(FLAG4REV)
endif

#
all: $(TARGET) $(MKBL1) $(TARGET).padding

$(TARGET):      $(OBJS)
	$(LD) -T $(BL1_LDS) -Ttext $(TEXT_BASE) -o $(O_TARGET) -Map BL1.map $(OBJS)
	$(OBJCOPY) -O binary $(O_TARGET) $(TARGET)

$(TARGET).padding:	$(TARGET) $(MKBL1)
	./$(MKBL1) $(TARGET) $(TARGET).padding

$(MKBL1):		mkbl1.c
	gcc $(CFLAGS_HOST) -o $(MKBL1) mkbl1.c

Main.o:			Main.c
	$(CC) $(CFLAGS) -c -o Main.o Main.c

Init.o:			Init.s
	$(AS) $(AFLAGS) -o Init.o Init.s

dep:
	gccmakedep $(OBJS:.o=.c)(OBJS:.o=.s)

clean   :
	rm -rf $(OBJS) $(TARGET) $(O_TARGET) BL1.map $(TARGET).padding $(MKBL1)

# DO NOT DELETE

