obj-m += gps.o

KDIR = /usr/src/kernel

all:
	make ARCH=arm -C $(KDIR) M=$(PWD) modules

clean:
	make ARCH=arm -C $(KDIR) M=$(PWD) clean