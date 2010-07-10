FLAGS = -Wall -static # -Ttext=0x100000
LIBS  =
EXEC  = fork_test

all: fork_test
	./diet gcc $(FLAGS) $(LIBS) fork_test.o -o $(EXEC)
	sudo cp $(EXEC) /home/wspeirs/moose/mnt/bin/$(EXEC)

fork_test.o: fork_test.c
	gcc -c $(FLAGS) fork_test.c

clean:
	rm *~ *.o $(EXEC)
