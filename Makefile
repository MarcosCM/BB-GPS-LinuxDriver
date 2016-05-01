obj-m += gps.o

KDIR = /usr/src/kernel

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean