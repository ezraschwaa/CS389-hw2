
CPP = g++

cache.o:
	$(CPP) -c cache.h cache.cpp;

eviction.o:
	$(CPP) -c eviction.h eviction.cpp;

cache:
	$(CPP) -g types.h book.h cache.cc eviction.cc -o test tests.cc;
	gdb ./test;

clean:
	rm -f *.o; rm -f *.h.gch; rm test
