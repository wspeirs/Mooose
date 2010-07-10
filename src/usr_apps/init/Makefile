FLAGS = -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -nostdinc -Ttext=0x500000
LIBS  =
EXEC  = init
INCLUDE = -I /home/wspeirs/moose/dietlibc-0.30/include

# do this in 1 shot for testing purposes with diet libc

all: main.c
	./diet gcc $(FLAGS) $(LIBS) main.c -static -o $(EXEC)
	objdump --no-show-raw-ins -d $(EXEC) > $(EXEC).asm

install: all
	sudo cp ./init /home/wspeirs/moose/mnt/usr/bin/init

clean:
	rm *~ *.o $(EXEC)
