#ifndef TYPES_H
#define TYPES_H

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

typedef struct
{
    char *key;
    char *value;
} RequestHeader;

typedef struct
{
    const enum HTTP_METHOD method;
    const char *body;
    RequestHeader* headers;
} FetchOptions;

typedef struct
{
    int status_code;
    char *status_text;
    char *body;
    char *url;
    RequestHeader* headers;
    int total_headers;
} Response;

struct URLInfo
{
    char hostname[BUFFER_SIZE];
    char path[BUFFER_SIZE];
};

#endif // !TYPES_H

