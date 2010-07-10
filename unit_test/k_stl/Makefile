GPP     = /usr/bin/g++
GCC     = /usr/bin/gcc
FLAGS   = -W -Wall -pedantic -g -Wctor-dtor-privacy -Wold-style-cast -Woverloaded-virtual -DUNIT_TEST -fno-stack-protector -fno-threadsafe-statics -fno-rtti -fno-exceptions
LIBS    =
INCLUDE = -I ../../src/include/k_std 
EXEC    = unit_test

#all: main.o list_test.o vector_test.o map_test.o bit_vector_test.o string_test.o 
all: main.o tree_test.o list_test.o
	$(GPP) $(FLAGS) $(LIBS) main.o list_test.o tree_test.o -o $(EXEC)

main.o: main.cpp 
	$(GPP) -c $(FLAGS) main.cpp

tree_test.o: tree_test.cpp rb_tree.h
	$(GPP) -c $(INCLUDE) $(FLAGS) tree_test.cpp

list_test.o: list_test.cpp list.h
	$(GPP) -c $(INCLUDE) $(FLAGS) list_test.cpp

# vector_test.o: vector_test.cpp
# 	$(GPP) -c $(INCLUDE) $(FLAGS) vector_test.cpp
# 
map_test.o: map_test.cpp
	$(GPP) -c $(INCLUDE) $(FLAGS) map_test.cpp
# 
# bit_vector_test.o: bit_vector_test.cpp
# 	$(GPP) -c $(INCLUDE) $(FLAGS) bit_vector_test.cpp
# 

#string_test.o: string_test.cpp
#	$(GPP) -c $(INCLUDE) $(FLAGS) string_test.cpp

clean:
	rm *~ *.o $(EXEC)
