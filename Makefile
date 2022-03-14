#TOOLCHAIN=~/toolchain/gcc-arm-none-eabi-4_9-2014q4/bin
#PREFIX=$(TOOLCHAIN)/arm-none-eabi-
PREFIX=arm-none-eabi-

ARCHFLAGS=-mthumb -mcpu=cortex-m0plus
COMMONFLAGS=-g3 -O0 -Wall -Werror -D "CPU_MKL46Z256VLL4" $(ARCHFLAGS)

CFLAGS=-I./board -I./board/src -I./CMSIS -I./drivers -I./utilities $(COMMONFLAGS)
LDFLAGS=$(COMMONFLAGS) --specs=nano.specs -Wl,--gc-sections,-Map,led_blinky.map,-Tlink.ld
LDFLAGS2=$(COMMONFLAGS) --specs=nano.specs -Wl,--gc-sections,-Map,hello_world.map,-Tlink.ld

CC=$(PREFIX)gcc
LD=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy
SIZE=$(PREFIX)size
RM=rm -f

TARGET=led_blinky
TARGET2=hello_world
STARTU=startup

BOARDS=$(wildcard board/pin_mux.c)
OBJ1=$(patsubst %.c, %.o, $(BOARDS))
BOARDS2=$(wildcard board/pin_mux2.c)
OBJ8=$(patsubst %.c, %.o, $(BOARDS2))
BOARDSRCS=$(wildcard board/src/*.c)
OBJ2=$(patsubst %.c, %.o, $(BOARDSRCS))
CMSISS=$(wildcard CMSIS/*.c) 
OBJ3=$(patsubst %.c, %.o, $(CMSISS))
UTILSS=$(wildcard utilities/*.c)
OBJ4=$(patsubst %.c, %.o, $(UTILSS)) 
DRIVERSS=$(wildcard drivers/*.c)
OBJ5=$(patsubst %.c, %.o, $(DRIVERSS))
OBJ6=$(patsubst %.c, %.o, $(STARTU).c)
OBJ9=$(patsubst %.c, %.o, $(TARGET).c)
OBJ10=$(patsubst %.c, %.o, $(TARGET2).c)

all: build size \
     build2 size2
build: elf srec bin
build2: elf2 srec2 bin2
elf: $(TARGET).elf
srec: $(TARGET).srec
bin: $(TARGET).bin
elf2: $(TARGET2).elf
srec2: $(TARGET2).srec
bin2: $(TARGET2).bin

clean:
	$(RM) $(TARGET).srec $(TARGET).elf $(TARGET).bin $(TARGET).map $(TARGET2).srec $(TARGET2).elf $(TARGET2).bin $(TARGET2).map $(OBJ8) $(OBJ9) $(OBJ10) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6)


$(TARGET).elf: $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ9)
	$(LD) $(LDFLAGS) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ9) $(LDLIBS) -o $@

$(TARGET2).elf: $(OBJ8) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ10)
	$(LD) $(LDFLAGS2) $(OBJ8) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ10) $(LDLIBS) -o $@

$(TARGET).srec: $(TARGET).elf
	$(OBJCOPY) -O srec $< $@

$(TARGET2).srec: $(TARGET2).elf
	$(OBJCOPY) -O srec $< $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET2).bin: $(TARGET2).elf
	$(OBJCOPY) -O binary $< $@

size:
	$(SIZE) $(TARGET).elf

size2:
	$(SIZE) $(TARGET2).elf

flash_led: all
	openocd -f openocd.cfg -c "program $(TARGET).elf verify reset exit"

flash_hello: all
	openocd -f openocd.cfg -c "program $(TARGET2).elf verify reset exit"
