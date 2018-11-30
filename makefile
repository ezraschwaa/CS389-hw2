
CPP = g++-7

cache.o:
	$(CPP) -c cache.h cache.cpp;

eviction.o:
	$(CPP) -c eviction.h eviction.cpp;

cache:
	$(CPP) -g types.h book.h cache.cc eviction.cc -o test tests.cc;
	gdb ./test;

cache_server:
	$(CPP) -g types.h book.h cache.cc eviction.cc -o server server.cc;
	./server;

cache_client:
	$(CPP) -g cache.h -o client client.cc;
	./client;

cache_test:
	$(CPP) -g types.h book.h client.cc eviction.cc -o test tests.cc;
	./test;

clean:
	rm -f *.o; rm -f *.h.gch; rm test
