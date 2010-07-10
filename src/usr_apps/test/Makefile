FLAGS = -Wall -static # -Ttext=0x100000
LIBS  =
EXEC  = test

all: proc_a.o proc_b.o
	./diet gcc $(FLAGS) $(LIBS) proc_a.o -o proc_a
	./diet gcc $(FLAGS) $(LIBS) proc_b.o -o proc_b
	sudo cp proc_a proc_b /home/wspeirs/moose/mnt/bin/

proc_a.o: proc_a.c
	gcc -c $(FLAGS) proc_a.c

proc_b.o: proc_b.c
	gcc -c $(FLAGS) proc_b.c

clean:
	rm *~ *.o $(EXEC)
