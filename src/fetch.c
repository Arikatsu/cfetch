#include "fetch.h"

Response* fetch(const char *url, const FetchOptions *options)
{
    char *response = NULL;
    
    struct URLInfo url_info;
    
	if (parse_url(url, &url_info) != 0)
	{
		fprintf(stderr, "Invalid URL: %s\n", url);
		return NULL;
	}
    
	fprintf(stdout, "Hostname: %s\n", url_info.hostname);
	fprintf(stdout, "Path: %s\n", url_info.path);

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) 
    {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        return NULL;
    }
    
    struct addrinfo *server = NULL;
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

	Response *response_struct = (Response*)malloc(sizeof(Response));
	
	if (parse_http_response(response, response_struct) != 0)
	{
		fprintf(stderr, "Error parsing HTTP response\n");
		cleanup(sockfd, server, NULL);
		free(response);
		free(response_struct);
		return NULL;
	}
    response_struct->url = (char*)url;
    
    cleanup(sockfd, server, NULL);
    free(response);
    
	return response_struct;
}
