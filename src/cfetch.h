#ifndef C_FETCH
#define C_FETCH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 1024

enum HTTP_METHOD
{
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_HEAD,
    HTTP_OPTIONS,
    HTTP_TRACE,
    HTTP_CONNECT
};

typedef struct {
	char* key;
	char* value;
} RequestHeader;

typedef struct {
    const enum HTTP_METHOD method;
    const char* body;
    RequestHeader* headers;
} FetchOptions;

struct URLInfo {
    char hostname[BUFFER_SIZE];
	char path[BUFFER_SIZE];
};

void cleanup(SOCKET sockfd, struct addrinfo* server, char* error_message)
{
	if (sockfd != INVALID_SOCKET) {
		closesocket(sockfd);
	}
    if (server != NULL) {
        freeaddrinfo(server);
    }
	if (error_message != NULL) {
        perror(error_message);
	}
	WSACleanup();
}

char* http_get(SOCKET sockfd, struct addrinfo* server, struct URLInfo* url, const FetchOptions* options)
{
    char* response = NULL;
    
	char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n", url->path, url->hostname);

    if (options->headers != NULL) {
        for (int i = 0; options->headers[i].key != NULL; i++) {
            snprintf(request + strlen(request), BUFFER_SIZE - strlen(request), "%s: %s\r\n", options->headers[i].key, options->headers[i].value);
        }
    }
    
    snprintf(request + strlen(request), BUFFER_SIZE - strlen(request), "\r\n");

	fprintf(stderr, "Request:\n%s\n\n", request);

    int result = send(sockfd, request, strlen(request), 0);
    if (result == SOCKET_ERROR) {
        cleanup(sockfd, server, "Error sending request");
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    response = (char*)malloc(1);
    response[0] = '\0';

    int bytes_read;
    do {
        bytes_read = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            char* temp = NULL;
			if (response != '\0') {
				temp = realloc(response, strlen(response) + bytes_read + 1);
				if (temp == NULL) {
					fprintf(stderr, "Error: Memory reallocation failed\n");
					free(response);
					response = NULL;
					break;
				}
				response = temp;
			}
			else {
				response = (char*)malloc(bytes_read + 1);
			}
            response = temp;
            strcat(response, buffer);
        }
    } while (bytes_read > 0);

	if (bytes_read < 0) {
        cleanup(sockfd, server, "Error reading from socket");
        free(response);
        return NULL;
	}

    cleanup(sockfd, server, NULL);

    return response;
}

char* fetch(const char* url, const FetchOptions* options)
{
    char* response = NULL;
    
    struct URLInfo url_info;
    
    /* TODO: Implement a more robust URL parser that can handle different URL formats and schemes */
    if (sscanf(url, "http://%[^/]/%s", url_info.hostname, url_info.path) != 2) {
        fprintf(stderr, "Invalid URL: %s\n", url);
        return NULL;
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        return NULL;
    }
    
    struct addrinfo* server = NULL;
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    result = getaddrinfo(url_info.hostname, "http", &hints, &server);
    if (result != 0) {
        fprintf(stderr, "Error resolving hostname: %d\n", result);
        WSACleanup();
        return NULL;
    }
    
    SOCKET sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
    if (sockfd == INVALID_SOCKET) {
        cleanup(INVALID_SOCKET, server, "Error creating socket");
        return NULL;
    }
    
    result = connect(sockfd, server->ai_addr, (int)server->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cleanup(sockfd, server, "Error connecting to server");
        return NULL;
    }
    
	switch (options->method) {
	    case HTTP_GET:
			fprintf(stderr, "GET %s\n\n", url);
		    response = http_get(sockfd, server, &url_info, options);
			break;
            
	    default:
		    fprintf(stderr, "Error: Unsupported HTTP method\n");
		    cleanup(sockfd, server, NULL);
		    return NULL;
	}

    return response;
}

#endif /* C_FETCH */
