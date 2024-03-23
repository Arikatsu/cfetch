#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "types.h"

#pragma comment(lib, "Ws2_32.lib")

#define MAX_HEADERS 100

void cleanup(SOCKET sockfd, const char *error_message);

int parse_url(const char *url, struct URLInfo *url_info);

int parse_http_response(const char *response_text, Response *response);

int check_ipv4(const char* input);

#endif // !UTILS_H

