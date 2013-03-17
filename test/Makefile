# Makefile for allocator test program
# 
# Feb 2013 - Bryant Moscon


alloc_test: alloc_test.o
	g++ -g -o alloc_test alloc_test.o

alloc_test.o: alloc_test.cpp ../src/allocator.hpp
	g++ -c -g alloc_test.cpp

clean:
	rm alloc_test alloc_test.o
