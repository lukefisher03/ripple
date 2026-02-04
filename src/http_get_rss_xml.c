#include "http_get_rss_xml.h"
#include "logger.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <openssl/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <uriparser/Uri.h>
#include <netdb.h>

typedef struct host_and_path {
    char *host;
    char *path;
} host_and_path;

static int parse_url(const char *url, host_and_path *hp);

struct  ssl_connection {
    SSL_CTX  *ctx; // SSL context
    SSL      *ssl; // SSL object
    int       sfd; // Attached file descriptor
};

static struct ssl_connection *_ssl_connect(struct addrinfo *results, const char *host) {
    struct addrinfo *result_pointer = results; // Hold the results from getaddrinfo
    int socket_file_descriptor; // Socket file descriptor

    while (result_pointer != NULL) { // Iterate through the results linked list
        // This code is compatible with both IPV4 and IPV6
        socket_file_descriptor = socket(result_pointer->ai_family, result_pointer->ai_socktype, result_pointer->ai_protocol);
        if (socket_file_descriptor == -1) {
            continue;
        }

        if (connect(socket_file_descriptor, result_pointer->ai_addr, result_pointer->ai_addrlen) != -1) {
            break;
        }

        close(socket_file_descriptor);
        result_pointer = result_pointer->ai_next;
    }

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        log_debug("SSL_CTX_new failed!");
        close(socket_file_descriptor);
        return NULL;
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket_file_descriptor); // Bind the ssl socket object to the socket fd.
    SSL_set_tlsext_host_name(ssl, host); // Some servers won't accept connections without this.
   
    int e;
    if ((e = SSL_connect(ssl)) <= 0) {
        e = SSL_get_error(ssl, e);
        log_debug("TLS Handshake failed! %d\n", e);
        log_debug("ERR: %s", ERR_error_string(e, NULL));
        close(socket_file_descriptor);
        return NULL;
    }

    struct ssl_connection *ssl_items = malloc(sizeof(*ssl_items));
    
    if (!ssl_items) {
        log_debug("Failed to allocate SSL connection struct!\n");
        return NULL;
    }
    
    ssl_items->ctx = ctx;
    ssl_items->ssl = ssl;
    ssl_items->sfd = socket_file_descriptor;

    return ssl_items;
}

static char * request_rss_xml(struct ssl_connection *ssl_items, const char * host, const char * path) {
    size_t bytes_read = 0;
    size_t cap = 64000;

    char *response = malloc(cap);
    char request[1024]; // TODO: Investigate if this is the proper size for this buffer

    snprintf(request, sizeof(request), 
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8\r\n"
                "Accept-language: en-US,en;q=0.8\r\n"
                "upgrade-insecure-requests: 1\r\n"
                "user-agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\r\n"
                "\r\n",
            path, host);

    int ret_code;
    if ((ret_code = SSL_write(ssl_items->ssl, request, strlen(request))) <= 0) {
        int error_code = SSL_get_error(ssl_items->ssl, ret_code);
        log_debug("Could not send get request! %s\n", ERR_error_string(error_code, NULL));
        return NULL;
    }

    ssize_t bytes;
    while((bytes = SSL_read(ssl_items->ssl, response + bytes_read, cap - bytes_read - 1)) > 0) {
        if (bytes <= 0) {
            int error_code = SSL_get_error(ssl_items->ssl, bytes);
            log_debug("Encountered an error when reading response, %s\n", ERR_error_string(error_code, NULL));
            return NULL;
        }
        bytes_read += bytes;
        if (bytes_read >= cap - 1) {
            cap *= 2; // Amortized O(1) append
            char *tmp = realloc(response, cap);
            if (tmp) {
                response = tmp;
            } else {
                log_debug("Memory error, failed to allocate new space for response.\n");
                free(response);
                return NULL;
            }
        }
    }

    response[bytes_read] = '\0';
    return response;
}

char * get_feed_xml(char *url) {
    host_and_path hp;
    if (parse_url(url, &hp) != 0) {
        log_debug("Failed to parse url, skipping");
        return NULL;
    }
    log_debug("%s, %s", hp.host, hp.path);
    int                 s; // Socket file descriptor and status variable for getaddrinfo.
    struct addrinfo     hints; // Narrow down results of getaddrinfo.
    struct addrinfo     *results;  // Pointer to head of linked list in which getaddrinfo will
                                        // store the returned results. `rp` is a pointer to traverse
                                        // the list.
    
    SSL_library_init();
    SSL_load_error_strings();
    
    // Set hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    // Run getaddrinfo with the "http" service
    s = getaddrinfo(hp.host, "https", &hints, &results);

    if (s != 0) {
        fprintf(stderr, "getaddrinfo error: %d %s\n", s, gai_strerror(s));
        return NULL;
    }

    struct ssl_connection *ssl = _ssl_connect(results, hp.host);
    freeaddrinfo(results);
    char * rss = request_rss_xml(ssl, hp.host, hp.path);
   
    SSL_shutdown(ssl->ssl);
    SSL_free(ssl->ssl);
    SSL_CTX_free(ssl->ctx);
    close(ssl->sfd);
    free(ssl);
    if (!rss) {
        log_debug("Failed to get RSS XML for feed url: %s", hp.host);
    }
    return rss;
}

void free_host_and_path(host_and_path *hp) {
    free(hp->host);
    free(hp->path);

    hp->host = NULL;
    hp->path = NULL;
}

static char *cp_range_to_buffer(UriTextRangeA *rng, size_t max_len) {
    if (!rng) return NULL;
    size_t len = (size_t)(rng->afterLast - rng->first);
    log_debug("Host len: %lu", len);
    if (len > max_len) {
        log_debug("Url domain is too long");
        return NULL;
    } 

    char *buf = malloc(len + 1);
    if (!buf) {
        log_debug("Failed to allocate space for url host");
        return NULL;
    }

    memcpy(buf, rng->first, len);
    buf[len] = '\0';
    return buf;
}

static int parse_url(const char *url, host_and_path *hp) {
    if (!url) return 1;

    UriUriA uri;
    const char *errorPos; 
    
    if (uriParseSingleUriA(&uri, url, &errorPos) != URI_SUCCESS) {
        log_debug("Failed to parse uri: %s", url);
        log_debug("Error: %s", errorPos);
        return 1;
    }
    if (!uri.hostText.first) {
        log_debug("Couldn't parse url, missing scheme (https, http) - %s", url);
        uriFreeUriMembersA(&uri);
        return 1;
    }
    hp->host = cp_range_to_buffer(&uri.hostText, MAX_DOMAIN_LEN);
    hp->path = malloc(MAX_PATH_LEN);
    int ret = snprintf(hp->path, MAX_PATH_LEN, "%s", uri.hostText.afterLast); 

    if (!hp->host || ret < 0 || ret >= MAX_PATH_LEN) {
        log_debug("Failed to parse url - %s\n", url);
        free_host_and_path(hp);
        uriFreeUriMembersA(&uri);
        return 1;
    }
    log_debug("%s", hp->host); 

    return 0;
}