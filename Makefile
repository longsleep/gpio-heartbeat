obj-m+=gpio-heartbeat.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
install:
	cp -a gpio-heartbeat.ko /lib/modules/$(shell uname -r)/kernel/extramodules
	depmod -a
