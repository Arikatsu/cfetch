#ifndef FETCH_H
#define FETCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "utils.h"
#include "methods.h"
#include "types.h"

#pragma comment(lib, "Ws2_32.lib")

Response* fetch(const char *url, const FetchOptions *options);

#endif // !FETCH_H
