KERNEL_DIR=../../ebf-buster-linux/build_image/build
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH CROSS_COMPILE
HOST_IP ?= 192.168.31.183

obj-m := hellomodule.o calculation.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

.PHONE:clean copy

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

copy:
	scp *.ko debian@$(HOST_IP):/home/debian/