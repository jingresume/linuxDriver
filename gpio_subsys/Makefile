KERNEL_DIR=../../ebf-buster-linux/build_image/build
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH CROSS_COMPILE
HOST_IP ?= 192.168.31.247

obj-m := rgb_led_gpio.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

.PHONE:clean copy

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

copy:
	scp *.ko $(out) debian@$(HOST_IP):/home/debian/