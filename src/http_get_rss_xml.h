#ifndef HTTP_GET_RSS_XML_H
#define HTTP_GET_RSS_XML_H

#define MAX_DOMAIN_LEN 255
#define MAX_PATH_LEN 4096

#include <stdlib.h>

char * get_feed_xml(char *url, size_t *size);

#endif