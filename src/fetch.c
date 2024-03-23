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

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) 
    {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        return NULL;
    }
    
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) 
    {
        cleanup(INVALID_SOCKET, "Error creating socket");
        return NULL;
    }

    if (check_ipv4(url_info.hostname) == 0)
    {
        struct sockaddr_in server = {
            .sin_family = AF_INET,
            .sin_port = htons(url_info.port),
        };

        if (inet_pton(AF_INET, url_info.hostname, &server.sin_addr) != 1)
        {
            cleanup(INVALID_SOCKET, "Error converting IP address");
            return NULL;
        }

        result = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
        if (result == SOCKET_ERROR)
        {
            cleanup(sockfd, NULL, "Error connecting to server");
            return NULL;
        }
    }
    else
    {
		struct addrinfo hints = {
			.ai_family = AF_INET,
			.ai_socktype = SOCK_STREAM,
		};

		struct addrinfo* res = NULL;
		result = getaddrinfo(url_info.hostname, url_info.scheme, &hints, &res);
		if (result != 0)
		{
			cleanup(sockfd, "Error getting address info");
			return NULL;
		}

		struct sockaddr_in server = {
			.sin_family = AF_INET,
			.sin_port = htons(url_info.port),
		};

		memcpy(&server.sin_addr, &((struct sockaddr_in*)res->ai_addr)->sin_addr, sizeof(struct in_addr));
		freeaddrinfo(res);

		result = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
		if (result == SOCKET_ERROR)
		{
			cleanup(sockfd, "Error connecting to server");
			return NULL;
		}
    }

    u_long mode = 1;
    result = ioctlsocket(sockfd, FIONBIO, &mode);
    if (result != NO_ERROR)
    {
        cleanup(sockfd, "Error setting socket to non-blocking mode");
        return NULL;
    }
    
	switch (options->method) 
    {
	    case HTTP_GET:
			fprintf(stderr, "GET %s\n\n", url);
		    response = http_get(sockfd, &url_info, options);
			break;
            
	    default:
		    fprintf(stderr, "Error: Unsupported HTTP method\n");
		    cleanup(sockfd, NULL);
		    return NULL;
	}

	Response *response_struct = (Response*)malloc(sizeof(Response));
	
	if (parse_http_response(response, response_struct) != 0)
	{
		cleanup(sockfd, "Error parsing HTTP response\n");
		free(response);
		free(response_struct);
		return NULL;
	}
    
    response_struct->url = (char*)url;
    
    cleanup(sockfd, NULL);
    free(response);
    
	return response_struct;
}
