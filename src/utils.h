#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "types.h"

#pragma comment(lib, "Ws2_32.lib")

#define MAX_HEADERS 100

void cleanup(const SOCKET sockfd, const struct addrinfo *server, const char *error_message);

int parse_url(const char *url, struct URLInfo *url_info);

int parse_http_response(const char *response_text, Response *response);

#endif // !UTILS_H

