// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cassert>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cache.h"

uint16_t PORTNO = 33052;
const char *ipAddress = "127.0.0.1";

cache_type create_cache(index_type maxmem, hash_func hasher) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientSocket >= 0 && "Socket creation failed.");

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORTNO);
    int addressSuccess = inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr.s_addr);
    assert(addressSuccess == 1 && "Socket address set failed.");

    const sockaddr *addressPointer = reinterpret_cast<const sockaddr*>(&serverAddress);
    int connectionSuccess = connect(clientSocket, addressPointer, sizeof(serverAddress));
    assert(connectionSuccess == 0 && "Socket connection failed.");

    // Message server to construct what needs constructing

    // return &clientSocket;
}

val_type cache_get(cache_type cache, key_type key, index_type *val_size) {
    const char* requestPart1 = "GET /key/";
    const char* requestPart2 = "\n\n";
    const uint32_t msgSizePart1 = strlen(requestPart1);
    const uint32_t msgSizePart2 = (strlen(requestPart2) + 1); //+1 for null terminator
    uint32_t keySize = (strlen(key));

    char request[msgSizePart1 + keySize + msgSizePart2];
    assert(request != NULL && "Failed to allocate memory in GET request.");
    strcpy(request, requestPart1);
    strcpy(&request[msgSizePart1], key);
    strcpy(&request[msgSizePart1 + keySize], requestPart2);
    request[msgSizePart1 + keySize + msgSizePart2 - 1] = 0;

    std::cout << request;

    // Send request to server
    // Receive reply from server

    return NULL; //Placeholder
}

int main() {
    cache_type placeholderCache;
    key_type shortCstring = "bbq";
    index_type placeholderValSize;
    cache_get(placeholderCache, shortCstring, &placeholderValSize);
    return 1; // Main is for testing only, will vanish from final program
}