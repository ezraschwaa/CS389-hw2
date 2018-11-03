//By Monica Moniot and Alyssa Riceman
#include "cache.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

using byte = uint8_t;//this must have the size of a unit of memory (a byte)
using uint_ptr = uint64_t;//this must have the size of a pointer

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using uint32 = index_type;
using Hash_func = hash_func;


constexpr int PORT = 8765;

int main(int argc, char const *argv[])
{
	uint32 server_fd;
	uint32 addrlen;
	sockaddr_in address;
	byte buffer[1024];
	{
		addrlen = sizeof(address);

		// Creating socket file descriptor
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(server_fd == 0) {
			perror("socket failed");
			return -1;
		}

		// Forcefully attaching socket to the port
		uint32 opt = 1;
		bool failure = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
		if(failure) {
			perror("setsockopt");
			return -1;
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(PORT);

		// Forcefully attaching socket to the port
		auto error_code = bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
		if(error_code < 0) {
			perror("bind failed");
			return -1;
		}
		error_code = listen(server_fd, 3);
		if(error_code < 0) {
			perror("listen");
			return -1;
		}
	}


	while(true) {
		uint32 new_socket = accept(server_fd, reinterpret_cast<sockaddr*>(&address), reinterpret_cast<socklen_t*>(&addrlen));
		if(new_socket < 0) {
			perror("accept");
			return -1;
		}

		const char* hello = "hello world";

		uint32 val_read = read(new_socket , buffer, 1024);
		printf("%s\n", buffer);

		send(new_socket, hello, strlen(hello), 0);
		printf("Hello message sent\n");
	}
	return 0;
}
