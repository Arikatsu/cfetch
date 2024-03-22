#include "methods.h"

char* http_get(const SOCKET sockfd, const struct addrinfo *server, const struct URLInfo *url, const FetchOptions *options)
{
    char *response = NULL;

    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n", url->path, url->hostname);

    if (options->headers != NULL)
    {
        for (int i = 0; options->headers[i].key != NULL; i++)
        {
            snprintf(request + strlen(request), BUFFER_SIZE - strlen(request), "%s: %s\r\n", options->headers[i].key, options->headers[i].value);
        }
    }

    snprintf(request + strlen(request), BUFFER_SIZE - (int)strlen(request), "\r\n");

    int result = send(sockfd, request, (int)strlen(request), 0);
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
        result = select((int)sockfd + 1, &read_fds, NULL, NULL, &timeout);
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
                temp = (char*)realloc(response, strlen(response) + bytes_read + 1);
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