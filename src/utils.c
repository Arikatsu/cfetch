#include "utils.h"

void cleanup(SOCKET sockfd, const char *error_message)
{
    if (sockfd != INVALID_SOCKET)
        closesocket(sockfd);

    if (error_message != NULL)
        perror(error_message);

    WSACleanup();
}

int parse_url(const char *url, struct URLInfo *url_info)
{
    char *temp = (char*)malloc(strlen(url) + 1);
    if (temp == NULL)
    {
        perror("Error: memory allocation failed\n");
        return -1;
    }
    strcpy(temp, url);

    char *scheme = strtok(temp, "://");
    if (scheme == NULL)
    {
        perror("Error: scheme not found\n");
        free(temp);
        return -1;
    }

    if (scheme == "http" || scheme == "https")
    {
        perror("Error: invalid scheme\n");
        free(temp);
        return -1;
    }
	strcpy(url_info->scheme, scheme);

    char *host = strtok(NULL, "/");
    if (host == NULL)
    {
        perror("Error: host not found\n");
        free(temp);
        return -1;
    }
	
	if (strchr(host, ':') != NULL)
	{
		char* hostname = strtok(host, ":");
		if (hostname == NULL)
		{
			perror("Error: hostname not found\n");
			free(temp);
			return -1;
		}
		strcpy(url_info->hostname, hostname);

		char* port = strtok(NULL, "/");
		if (port == NULL)
		{
			perror("Error: port not found\n");
			free(temp);
			return -1;
		}
		url_info->port = atoi(port);
	}
	else
	{
		strcpy(url_info->hostname, host);
		
		if (strcmp(scheme, "http") == 0)
		{
			url_info->port = 80;
		}
		else if (strcmp(scheme, "https") == 0)
		{
			url_info->port = 443;
		}
	}

    char *temp_path = strtok(NULL, "");
    if (temp_path == NULL)
    {
        strcpy(url_info->path, "/");
    }
    else
    {
        char *path;
        path = (char*)malloc(strlen(temp_path) + 2);
        if (path == NULL)
        {
            perror("Failed to allocate memory for path");
            free(temp);

            return -1;
        }
        strcpy(path, "/");
        strcat(path, temp_path);
        strcpy(url_info->path, path);
    }

    free(temp);
    return 0;
}

int parse_http_response(const char *response_text, Response *response)
{
    char *line;
    char *token;
    const char *delim = "\r\n";

    response->status_code = 0;
    response->status_text = NULL;
    response->body = NULL;
    response->url = NULL;
    response->headers = NULL;
    response->total_headers = 0;

    line = strtok((char*)response_text, delim);
    char *temp = strtok(NULL, "");

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

    char *body_start = strstr(temp, "\r\n\r\n");
    if (body_start != NULL)
    {
        body_start += 4; // Skip "\r\n\r\n"
        char* body = _strdup(body_start);

        for (int i = (int)strlen(body) - 1; i >= 0; i--)
        {
            if (body[i] == '\r' || body[i] == '\n')
                body[i] = '\0';
            else
                break;
        }

        response->body = body;
    }

    char *header_start = temp;
    for (int i = 0; i <= MAX_HEADERS; i++)
    {
        char* header_end = strstr(header_start, "\r\n");
        if (header_end == NULL) {
            perror("Error: header_end not found\n");
            return -1;
        }

        char *header = (char*)malloc(header_end - header_start + 1);
        if (header == NULL) {
            printf("Error: memory allocation failed\n");
            return -1;
        }

        strncpy(header, header_start, header_end - header_start);
        header[header_end - header_start] = '\0';

        char *key = strtok(header, ":");
        char *value = strtok(NULL, "");

        key = strtok(key, "\n");

        if (key == NULL)
        {
            free(header);
            break;
        }

        response->headers = (RequestHeader*)realloc(response->headers, (i + 1) * sizeof(RequestHeader));
        if (response->headers == NULL)
        {
            perror("Error reallocating memory for headers");
            return -1;
        }

        response->headers[i].key = _strdup(key);
        response->headers[i].value = _strdup(value);

        response->total_headers++;

        header_start = header_end + 2;
        free(header);
    }

    return 0;
}

int check_ipv4(const char* input) {
    int has_digit = 0;
    int has_alpha = 0;
    int has_dot = 0;
    
    while (*input) {
        if (isdigit((unsigned char)*input)) 
            has_digit = 1;
        else if (isalpha((unsigned char)*input) || *input == '-') 
            has_alpha = 1;
        else if (*input == '.') 
            has_dot = 1;
        else 
            return -1;
        
        input++;
    }
    
    if (has_digit && has_dot && !has_alpha) 
        return 0;
    else
        return 1;
}