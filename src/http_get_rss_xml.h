#ifndef HTTP_GET_RSS_XML_H
#define HTTP_GET_RSS_XML_H

#define MAX_DOMAIN_LEN 255
#define MAX_PATH_LEN 4096
#define MAX_REQUEST_RESPONSE_SIZE (10 * 1024 * 1024)

#include <stdlib.h>

typedef struct {
    char    *raw;
    char    *body;
    size_t  size;
    size_t  body_size;
} http_response;

http_response *send_http_get(char *url);
void free_http_response(http_response *response);

#endif