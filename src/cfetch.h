#ifndef C_FETCH
#define C_FETCH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 1024
#define TIMEOUT_MS 5000
#define MAX_HEADERS 100

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

typedef struct 
{
	char* key;
	char* value;
} RequestHeader;

typedef struct 
{
    const enum HTTP_METHOD method;
    const char* body;
    RequestHeader* headers;
} FetchOptions;

typedef struct
{
	int status_code;
    char* status_text;
	char* body;
	char* url;
	RequestHeader* headers;
	int total_headers;
} Response;

struct URLInfo 
{
    char hostname[BUFFER_SIZE];
	char path[BUFFER_SIZE];
};

void cleanup(SOCKET sockfd, struct addrinfo* server, char* error_message)
{
	if (sockfd != INVALID_SOCKET) 
		closesocket(sockfd);
    
    if (server != NULL) 
        freeaddrinfo(server);
    
	if (error_message != NULL) 
        perror(error_message);
    
	WSACleanup();
}

int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (tolower(*s1) != tolower(*s2)) {
            return tolower(*s1) - tolower(*s2);
        }
        s1++;
        s2++;
    }
    return tolower(*s1) - tolower(*s2);
}

void parse_http_response(char* response_text, Response* response) 
{
    char* line;
    char* token;
    const char* delim = "\r\n";
    
    response->status_code = 0;
    response->status_text = NULL;
    response->body = NULL;
    response->url = NULL;
    response->headers = NULL;
    response->total_headers = 0;
    
    line = strtok(response_text, delim);
	char* temp = strtok(NULL, "");
    
    if (line != NULL) 
    {
        token = strtok(line, " ");
        token = strtok(NULL, " ");
        if (token != NULL) 
        {
            response->status_code = atoi(token);
            token = strtok(NULL, "");
            if (token != NULL)
                response->status_text = _strdup(token);
        }
    }
    
    char* body_start = strstr(temp, "\r\n\r\n");
    if (body_start != NULL) 
    {
        body_start += 4; // Skip "\r\n\r\n"
        char* body = _strdup(body_start);
        
		// Remove unneeded characters from the end of the body
		for (int i = strlen(body) - 1; i >= 0; i--)
		{
			if (body[i] == '\r' || body[i] == '\n')
				body[i] = '\0';
			else
				break;
		}

		response->body = body;
    }
    
	char* header_start = temp;
	for (int i = 0; i <= MAX_HEADERS; i++) 
    {
		char* header_end = strstr(header_start, "\r\n");
        if (header_end == NULL) {
            perror("Error: header_end not found\n");
            break;
        }
        
        char* header = (char*)malloc(header_end - header_start + 1);
        if (header == NULL) {
            printf("Error: memory allocation failed\n");
            break;
        }

		strncpy(header, header_start, header_end - header_start);
		header[header_end - header_start] = '\0';

		char* key = strtok(header, ":");
		char* value = strtok(NULL, "");

        // trim any \n from key
		key = strtok(key, "\n");
        
        if (key == NULL)
        {
            free(header);
			break;
        }

		// Reallocate memory for headers
		response->headers = (RequestHeader*)realloc(response->headers, (i + 1) * sizeof(RequestHeader));
		if (response->headers == NULL)
		{
			perror("Error reallocating memory for headers");
			break;
		}

		response->headers[i].key = _strdup(key);
		response->headers[i].value = _strdup(value);

		response->total_headers++;

		header_start = header_end + 2;
		free(header);
	}
}

char* http_get(SOCKET sockfd, struct addrinfo* server, struct URLInfo* url, const FetchOptions* options)
{
    char* response = NULL;
    
	char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n", url->path, url->hostname);

    if (options->headers != NULL) 
    {
        for (int i = 0; options->headers[i].key != NULL; i++) 
        {
            snprintf(request + strlen(request), BUFFER_SIZE - strlen(request), "%s: %s\r\n", options->headers[i].key, options->headers[i].value);
        }
    }
    
    snprintf(request + strlen(request), BUFFER_SIZE - strlen(request), "\r\n");

    int result = send(sockfd, request, strlen(request), 0);
    if (result == SOCKET_ERROR) 
    {
        int error_code = WSAGetLastError();
        if (error_code != WSAEWOULDBLOCK) {
            cleanup(sockfd, server, "Error sending request");
            return NULL;
        }
    }

    char buffer[BUFFER_SIZE];
    response = (char*)malloc(1);
    response[0] = '\0';
    
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_MS / 1000;
    timeout.tv_usec = (TIMEOUT_MS % 1000) * 1000;

    int bytes_read;
    do 
    {
        result = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        if (result < 0) 
        {
            cleanup(sockfd, server, "Error in select()");
            free(response);
            return NULL;
        }
        else if (result == 0) 
        {
            cleanup(sockfd, server, "Timeout occurred");
            free(response);
            return NULL;
        }
        
        bytes_read = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_read > 0) 
        {
            buffer[bytes_read] = '\0';
            char* temp = NULL;
			if (response != '\0') 
            {
				temp = realloc(response, strlen(response) + bytes_read + 1);
				if (temp == NULL) 
                {
					fprintf(stderr, "Error: Memory reallocation failed\n");
					free(response);
					response = NULL;
					break;
				}
				response = temp;
			}
			else 
            {
				response = (char*)malloc(bytes_read + 1);
			}
            response = temp;
            strcat(response, buffer);
        }
    } while (bytes_read > 0);

	if (bytes_read < 0) 
    {
        cleanup(sockfd, server, "Error reading from socket");
        free(response);
        return NULL;
	}
    
    return response;
}

Response* fetch(const char* url, const FetchOptions* options)
{
    char* response = NULL;
    
    struct URLInfo url_info;
    
    /* TODO: Implement a more robust URL parser that can handle different URL formats and schemes */
    if (sscanf(url, "http://%[^/]/%s", url_info.hostname, url_info.path) != 2) 
    {
        fprintf(stderr, "Invalid URL: %s\n", url);
        return NULL;
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) 
    {
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
    if (result != 0) 
    {
        cleanup(INVALID_SOCKET, NULL, "Error resolving hostname");
        return NULL;
    }
    
    SOCKET sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
    if (sockfd == INVALID_SOCKET) 
    {
        cleanup(INVALID_SOCKET, server, "Error creating socket");
        return NULL;
    }
    
    result = connect(sockfd, server->ai_addr, (int)server->ai_addrlen);
    if (result == SOCKET_ERROR) 
    {
        cleanup(sockfd, server, "Error connecting to server");
        return NULL;
    }

    u_long mode = 1;
    result = ioctlsocket(sockfd, FIONBIO, &mode);
    if (result != NO_ERROR)
    {
        cleanup(sockfd, server, "Error setting socket to non-blocking mode");
        return NULL;
    }
    
	switch (options->method) 
    {
	    case HTTP_GET:
			fprintf(stderr, "GET %s\n\n", url);
		    response = http_get(sockfd, server, &url_info, options);
			break;
            
	    default:
		    fprintf(stderr, "Error: Unsupported HTTP method\n");
		    cleanup(sockfd, server, NULL);
		    return NULL;
	}

	Response* response_struct = (Response*)malloc(sizeof(Response));
	parse_http_response(response, response_struct);
    response_struct->url = url;
    
    cleanup(sockfd, server, NULL);
    free(response);
    
	return response_struct;
}

#endif /* C_FETCH */
