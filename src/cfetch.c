#include <stdio.h>
#include "cfetch.h"

int main()
{
	/* goofy ahh testing */
	
	FetchOptions options = {
		.method = HTTP_GET,
		.body = NULL,
		.headers = (RequestHeader[]) {
			{ "User-Agent", "cfetch" },
			{ "Accept", "application/json" },
			{ "Content-Type", "application/json" },
			{ NULL, NULL }
		}
	};
	
	char* ok = fetch("http://httpbin.org/get", &options);

	printf("%s\n", ok);
    
    return 0;
}