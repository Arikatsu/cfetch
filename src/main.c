#include <stdio.h>

#include "fetch.h"

#define PRINT_RESPONSE(response)														\
	printf("Status Code: %d\n", response->status_code);									\
	printf("Status Text: %s\n", response->status_text);									\
	printf("Headers: \n");																\
	for (int i = 0; i < response->total_headers; i++)									\
		printf(" - %s: %s\n", response->headers[i].key, response->headers[i].value);	\
	printf("Total Headers: %d\n", response->total_headers);								\
	printf("Body: %s\n", response->body);												\
	printf("URL: %s\n", response->url);													\
	printf("\n");

int main()
{
	/* goofy ahh testing */
	
	FetchOptions options = 
	{
		.method = HTTP_GET,
		.body = NULL,
		.headers = (RequestHeader[]) 
		{
			{ "User-Agent", "cfetch" },
			{ "Accept", "application/json" },
			{ "Content-Type", "application/json" },
			{ NULL, NULL }
		}
	};
	
	Response *ok = fetch("https://127.0.0.1:3000/", &options);
	
	if (ok == NULL) 
	{
		printf("Failed to fetch\n");
		free(ok);
		return 1;
	}
	
	PRINT_RESPONSE(ok);
	free(ok);
    
    return 0;
}