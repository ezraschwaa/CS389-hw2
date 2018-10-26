
CPP = g++-7

cache.o:
	$(CPP) -c cache.h cache.cpp;

eviction.o:
	$(CPP) -c eviction.h eviction.cpp;

cache_face.o:
	$(CPP) -c cache.hh cache.h cache_face.cpp

cache: cache.o eviction.o
	$(CPP) -O4 types.h book.h cache.o eviction.o tests.cc -o test;

cache_debug: cache.o eviction.o cache_face.o
	$(CPP) -g types.h book.h cache.hh cache.h cache.o eviction.o cache_face.o tests.cc -o test;
	# gdb ./test;

clean:
	rm -f *.o; rm -f *.h.gch; rm test
