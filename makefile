
CPP = g++

cache.o:
	$(CPP) -c cache.h cache.cpp;

eviction.o:
	$(CPP) -c eviction.h eviction.cpp;

cache: cache.o eviction.o
	$(CPP) -O4 types.h book.h cache.o eviction.o tests.cc -o test;
cache_debug: cache.o eviction.o
	$(CPP) -g types.h book.h cache.o eviction.o tests.cc -o test;
	gdb ./test;

clean:
	rm -f *.o; rm -f *.h.gch; rm test
