#ifndef METHODS_H
#define METHODS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "utils.h"

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 1024
#define TIMEOUT_MS 5000

char* http_get(const SOCKET sockfd, const struct addrinfo* server, const struct URLInfo* url, const FetchOptions* options);

#endif // !METHODS_H
