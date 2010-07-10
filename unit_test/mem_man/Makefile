GPP     = /usr/bin/g++
GCC     = /usr/bin/gcc
FLAGS   = -g -O2 -pedantic -W -Wall -Wcast-align -Winline -Wredundant-decls -Wpointer-arith -fno-ident -Wctor-dtor-privacy -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wcast-qual -Wunused-macros -Wno-format -Wno-unused-variable -Wno-long-long -D UNIT_TEST
LIBS    =
INCLUDE = -I ../../src/include -I ../../src/include/k_std
EXEC    = mem_test

all: mem_manage.o main.o
	$(GPP) $(FLAGS) $(LIBS) *.o -o $(EXEC)

mem_manage.o: mem_manage.cpp *.h
	$(GPP) -c $(INCLUDE) $(FLAGS) -DUNIT_TEST mem_manage.cpp

main.o: main.cpp *.h
	$(GPP) -c $(INCLUDE) $(FLAGS) main.cpp

clean:
	rm *~ *.o $(EXEC)
