KERNEL_DIR=../../ebf-buster-linux/build_image/build
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH CROSS_COMPILE
HOST_IP ?= 192.168.31.
ip := $(value ip)

obj-m := spi_oled.o
out   := test

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	$(CROSS_COMPILE)gcc -g -o $(out) test.c 

.PHONE:clean copy

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

copy:
	scp *.ko $(out) debian@$(HOST_IP)$(ip):/home/debian/