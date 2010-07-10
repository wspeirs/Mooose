GPP     = /usr/bin/g++
GCC     = /usr/bin/gcc
FLAGS   = -g -pedantic -W -Wall -Wcast-align -Winline -Wredundant-decls -Wpointer-arith -fno-ident -Wctor-dtor-privacy -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wcast-qual -Wunused-macros -Wno-format -Wno-unused-variable -Wno-long-long -D UNIT_TEST
LIBS    =
INCLUDE = -I ../../src/include -I ../../src/include/k_std
EXEC    = test

all: main.o ext2.o FileSystemManager.o
	$(GPP) $(FLAGS) $(LIBS) *.o -o $(EXEC)
#	-sudo umount /mnt/floppy
#	cp backup.img disk.img
#	sync
#	sudo mount -o loop disk.img /mnt/floppy

ext2.o: ext2.cpp
	$(GPP) -c $(INCLUDE) $(FLAGS) ext2.cpp

FileSystemManager.o: FileSystemManager.cpp
	$(GPP) -c $(INCLUDE) $(FLAGS) FileSystemManager.cpp

main.o: main.cpp *.h
	$(GPP) -c $(INCLUDE) $(FLAGS) main.cpp

clean:
	rm *~ *.o $(EXEC)
