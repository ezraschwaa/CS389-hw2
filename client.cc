// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cassert>
#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include "cache.h"

typedef int socketType;
struct cache_obj {
    socketType tcpSocket;
    socketType udpSocket;
};

uint16_t PORTNUM = 33052;
const char* LOCALHOST = "127.0.0.1";

char* makeHttpRequest(const char* method, const char* uriBase, const char* key, const char* value) {
    uint32_t beforeKeyLength = strlen(method) + strlen(uriBase) + 2; //Add 2 for the " /" between the method and the URI
    uint32_t keyLength = 0;
    uint32_t valLength = 0;
    if (key != NULL) {
        keyLength = strlen(key) + 1; //Add 1 for the / before the key
    }
    if (value != NULL) {
        valLength = strlen(value) + 1; //Add 1 for the / before the value
    }
    uint32_t overallLength = beforeKeyLength + keyLength + valLength + 3; //Add 3 for two newlines and one null terminator

    char* request = new char[overallLength];
    assert(request != NULL && "Failed to allocate memory for HTTP request.");
    const char* finalNewlines = "\n\n";
    uint32_t requestFillSuccess = 0;
    if ((key == NULL) && (value == NULL)) {
        requestFillSuccess = sprintf(request, "%s /%s%s", method, uriBase, finalNewlines);
    } else if ((key != NULL) && (value == NULL)) {
        requestFillSuccess = sprintf(request, "%s /%s/%s%s", method, uriBase, key, finalNewlines);
    } else if ((key == NULL) && (value != NULL)) {
        requestFillSuccess = sprintf(request, "%s /%s/%s%s", method, uriBase, value, finalNewlines);
    } else { //(key != NULL) && (value != NULL)
        requestFillSuccess = sprintf(request, "%s /%s/%s/%s%s", method, uriBase, key, value, finalNewlines);
    }
    assert(requestFillSuccess > 0 && "Failed to stitch together HTTP request string.");

    return request;
}

int charsBeforeNewline(char* cStr) {
    int stringLength = strlen(cStr);
    for (int i = 0; i < stringLength; i++) {
        if (cStr[i] == '\n') {
            return i; 
        }
    }
    return stringLength;
}

char* getStatusCode(socketType clientSocket) {
    char serverReadBuffer[2048];
    int serverReadLength = read(clientSocket, serverReadBuffer, 2048);
    assert(serverReadLength >= 0 && "Status code not received from server.");

    int statusCodeStart = 0;
    while (statusCodeStart < serverReadLength) {
        if (serverReadBuffer[statusCodeStart] == ' '){
            statusCodeStart++;
            break;
        }
        statusCodeStart++;
    }
    int statusCodeEnd = charsBeforeNewline(serverReadBuffer);
    assert(statusCodeStart < statusCodeEnd && "Status code does not start where expected.");

    char* statusCodeStr = new char[statusCodeEnd - statusCodeStart];
    for (int i = 0; i < (statusCodeEnd - statusCodeStart); i++) {
        statusCodeStr[i] = serverReadBuffer[i + statusCodeStart];
    }
    std::cout << statusCodeStr << "\n";
    return statusCodeStr;
}

socketType start_socket(int commType, uint16_t portNum, const char* ipAddress) {
    socketType newSocket = socket(AF_INET, commType, 0);
    assert(newSocket >= 0 && "Socket creation failed.");

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNum);
    int addressSuccess = inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr.s_addr);
    assert(addressSuccess == 1 && "Socket address set failed.");

    const sockaddr* addressPointer = reinterpret_cast<const sockaddr *>(&serverAddress);
    int connectionSuccess = connect(newSocket, addressPointer, sizeof(serverAddress));
    assert(connectionSuccess == 0 && "Socket connection failed.");

    return newSocket;
}

cache_type create_cache(index_type maxmem, hash_func hasher) {
    socketType tcpSocket = start_socket(SOCK_STREAM, PORTNUM, LOCALHOST);
    socketType udpSocket = start_socket(SOCK_DGRAM, PORTNUM, LOCALHOST);

    char* memsizeCstr = reinterpret_cast<char*>(&maxmem);

    // Ignore memsize and hasher; the server API doesn't allow them to work any more

    cache_obj* cacheStruct = new cache_obj;
    cacheStruct->tcpSocket = tcpSocket;
    cacheStruct->udpSocket = udpSocket;

    return cacheStruct;
}

int cache_set(cache_type cache, key_type key, val_type val, index_type val_size) {
    char* setRequest = makeHttpRequest("PUT", "key", key, static_cast<const char*>(val));
    std::cout << setRequest;
    send(cache->tcpSocket, setRequest, strlen(setRequest), 0);

    int setSuccess = 0;
    char* statusCode;
    bool codeNeeded = true;
    while (codeNeeded) { // Loop waiting for status code
        statusCode = getStatusCode(cache->tcpSocket);
        switch(statusCode[0]) {
            case '1':
                delete[] statusCode;
                continue;
            case '2':
                codeNeeded = false;
            case '3':
                delete[] statusCode;
                continue;
            case '4':
                setSuccess = -1;
                codeNeeded = false;
            case '5':
                setSuccess = -1;
                codeNeeded = false;
        }
    }

    delete[] setRequest;
    delete[] statusCode;
    return setSuccess; //Placeholder
}

val_type cache_get(cache_type cache, key_type key, index_type *val_size) {
    char* getRequest = makeHttpRequest("GET", "key", key, NULL);
    std::cout << getRequest;
    send(cache->tcpSocket, getRequest, strlen(getRequest), 0);

//     int deleteSuccess = 0;
//     char* statusCode;
//     bool codeNeeded = true;
//     while (codeNeeded) { // Loop waiting for status code
//         statusCode = getStatusCode(cache->tcpSocket);
//         switch(statusCode[0]) {
//             case '1':
//                 delete[] statusCode;
//                 continue;
//             case '2':
//                 codeNeeded = false;
//             case '3':
//                 delete[] statusCode;
//                 continue;
//             case '4':
//                 deleteSuccess = -1;
//                 codeNeeded = false;
//             case '5':
//                 deleteSuccess = -1;
//                 codeNeeded = false;
//         }
//     }

    delete[] getRequest;
    return NULL; //Placeholder
}

int cache_delete(cache_type cache, key_type key) {
    char* deleteRequest = makeHttpRequest("DELETE", "key", key, NULL);
    std::cout << deleteRequest;
    send(cache->tcpSocket, deleteRequest, strlen(deleteRequest), 0);

    int deleteSuccess = 0;
    char* statusCode;
    bool codeNeeded = true;
    while (codeNeeded) { // Loop waiting for status code
        statusCode = getStatusCode(cache->tcpSocket);
        switch(statusCode[0]) {
            case '1':
                delete[] statusCode;
                continue;
            case '2':
                codeNeeded = false;
            case '3':
                delete[] statusCode;
                continue;
            case '4':
                deleteSuccess = -1;
                codeNeeded = false;
            case '5':
                deleteSuccess = -1;
                codeNeeded = false;
        }
    }

    delete[] deleteRequest;
    delete[] statusCode;
    return 0; //Placeholder
}

index_type cache_space_used(cache_type cache){
    char* spaceUsedRequest = makeHttpRequest("GET", "memsize", NULL, NULL);
    std::cout << spaceUsedRequest;
    send(cache->tcpSocket, spaceUsedRequest, strlen(spaceUsedRequest), 0);

    // Receive reply from server

    delete[] spaceUsedRequest;
    return 0; //Placeholder
}

void destroy_cache(cache_type cache) {
    char* destroyRequest = makeHttpRequest("POST", "shutdown", NULL, NULL);
    std::cout << destroyRequest;
    send(cache->tcpSocket, destroyRequest, strlen(destroyRequest), 0);

    // Receive error code from server

    delete[] destroyRequest;
}

int main() {
    cache_type testCache = create_cache(4096, NULL);
    key_type shortCstring = "bbq";
    // char* valueText = "BarbecueLettuce";
    // void* placeholderValue = static_cast<void*>(valueText);
    index_type placeholderValSize = 15;
    // cache_set(testCache, shortCstring, placeholderValue, placeholderValSize);
    cache_get(testCache, shortCstring, &placeholderValSize);
    cache_delete(testCache, shortCstring);
    cache_space_used(testCache);
    destroy_cache(testCache);
    return 1; // Main is for testing only, will vanish from final program
}