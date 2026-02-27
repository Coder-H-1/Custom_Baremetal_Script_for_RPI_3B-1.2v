CROSS_COMPILE ?= aarch64-none-elf-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

CFLAGS  := -O2 -ffreestanding -nostdlib -Wall -Wextra -march=armv8-a
LDFLAGS := -nostdlib -static -T linker.ld

SRCS = \
    boot/start.S boot/cstart.c \
    kernel/kernel.c kernel/mem.c kernel/shell.c \
    kernel/core/utils.c kernel/core/panic.c \
    drivers/uart/uart.c \
    drivers/sd/sdhost.c  drivers/sd/fat32.c \
    drivers/IO/gpio.c \
    drivers/usb/core/usb_core.c drivers/usb/hc/dwc2.c \
    drivers/usb/msc/usb_msc.c drivers/usb/hub/usb_hub.c \
    drivers/video/framebuffer.c drivers/video/mailbox.c 



OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.S=.o)

all: kernel8.img

kernel8.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

kernel8.img: kernel8.elf
	$(OBJCOPY) $< -O binary $@ 

clean:
	rm -f $(OBJS) kernel8.elf kernel8.img
