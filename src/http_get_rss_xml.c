#include "http_get_rss_xml.h"
#include "logger.h"

#include <string.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <openssl/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <uriparser/Uri.h>
#include <netdb.h>

typedef struct {
    char    *host;
    char    *path;
    int     secure;
    char    *port;
} host_and_path;


static int parse_url(const char *url, host_and_path *hp);
void free_host_and_path(host_and_path *hp);

typedef struct {
    SSL_CTX  *ctx; // SSL context
    SSL      *ssl; // SSL object
    int       sfd; // Attached file descriptor
} ssl_connection;

static ssl_connection *_secure_connect(struct addrinfo *results, const char *host) {
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
        log_debug("Failed to build SSL context!");
        close(socket_file_descriptor);
        return NULL;
    }

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        log_debug("Failed to create SSL object");
        SSL_CTX_free(ctx);
        close(socket_file_descriptor);
        return NULL;
    }
    SSL_set_fd(ssl, socket_file_descriptor); // Bind the ssl socket object to the socket fd.
    SSL_set_tlsext_host_name(ssl, host); // Some servers won't accept connections without this.
   
    int e;
    if ((e = SSL_connect(ssl)) <= 0) {
        log_debug("Failed to create SSL connection!");
        SSL_CTX_free(ctx);
        SSL_free(ssl);
        close(socket_file_descriptor);
        return NULL;
    }

    ssl_connection *ssl_items = malloc(sizeof(*ssl_items));
    
    if (!ssl_items) {
        SSL_CTX_free(ctx);
        SSL_free(ssl);
        close(socket_file_descriptor);
        return NULL;
    }
    
    ssl_items->ctx = ctx;
    ssl_items->ssl = ssl;
    ssl_items->sfd = socket_file_descriptor;

    return ssl_items;
}
static int _insecure_connect(struct addrinfo *results) {
    struct addrinfo *rp;
    int sfd;

    for (rp = results; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            return sfd;
        }
        close(sfd);
    }
    return -1;
}

static http_response *send_insecure_request(int sfd, char *request) {
    size_t bytes_read = 0;
    size_t cap = 64000;

    char *raw_response = malloc(cap);
    if (!raw_response) {
        return NULL;
    }

    int ret_code;
    if ((ret_code = write(sfd, request, strlen(request) + 1)) <= 0) {
        return NULL;
    }

    ssize_t bytes;
    while ((bytes = read(sfd, raw_response + bytes_read, cap - bytes_read - 1)) > 0) {
        if (bytes <= 0) {
            return NULL;
        }
        bytes_read += bytes;
        if (bytes_read >= MAX_REQUEST_RESPONSE_SIZE) {
            free(raw_response);
            return NULL;
        }

        if (bytes_read >= cap - 1) {
            cap *= 2;
            char *tmp = realloc(raw_response, cap);

            if (!tmp) {
                free(raw_response);
                return NULL;
            }

            raw_response = tmp;
        }
    }

    raw_response[bytes_read] = '\0';
    
    size_t offset = 0;
    for (; offset + 4 <= bytes_read && strncmp(raw_response + offset, "\r\n\r\n", 4) != 0; offset++);
    
    http_response *response = malloc(sizeof(*response));
    response->raw = raw_response;
    response->body = raw_response + offset + 4;
    response->size = bytes_read;
    response->body_size = bytes_read - (offset + 4);

    return response;
}

static http_response *send_secure_request(ssl_connection *ssl_items, char *request) {
    size_t bytes_read = 0;
    size_t cap = 64000;

    char *raw_response = malloc(cap);

    int ret_code;
    if ((ret_code = SSL_write(ssl_items->ssl, request, strlen(request) + 1)) <= 0) {
        return NULL;
    }

    ssize_t bytes;
    while((bytes = SSL_read(ssl_items->ssl, raw_response + bytes_read, cap - bytes_read - 1)) > 0) {
        if (bytes <= 0) {
            return NULL;
        }
        bytes_read += bytes;
        if (bytes_read >= MAX_REQUEST_RESPONSE_SIZE) {
            free(raw_response);
            return NULL;
        }
        if (bytes_read >= cap - 1) {
            cap *= 2; // Amortized O(1) append
            char *tmp = realloc(raw_response, cap);

            if (!tmp) {
                free(raw_response);
                return NULL;
            }

            raw_response = tmp;
        }
    }

    raw_response[bytes_read] = '\0';

    size_t offset = 0;
    for (; offset + 4 <= bytes_read && strncmp(raw_response + offset, "\r\n\r\n", 4) != 0; offset++);
    
    http_response *response = malloc(sizeof(*response));
    response->raw = raw_response;
    response->body = raw_response + offset + 4;
    response->size = bytes_read;
    response->body_size = bytes_read - (offset + 4);

    return response;
}

http_response *send_http_get(char *url) {
    log_debug("Sending HTTP connection to: %s", url);
    host_and_path hp;
    if (parse_url(url, &hp) != 0) {
        return NULL;
    }

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
    s = getaddrinfo(hp.host, hp.port, &hints, &results);
    if (s != 0) {
        return NULL;
    }
    
    char request[4096]; // TODO: Investigate if this is the proper size for this buffer
    int res = snprintf(request, sizeof(request), 
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8\r\n"
    "Accept-language: en-US,en;q=0.8\r\n"
    "upgrade-insecure-requests: 1\r\n"
    "user-agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\r\n"
    "\r\n",
    hp.path, hp.host);

    if (res < 0 || res >= sizeof(request)) {
        return NULL;
    }
    
    http_response *response = NULL;

    if (hp.secure) {
        ssl_connection *ssl = _secure_connect(results, hp.host);
        if (!ssl) goto cleanup;
        response = send_secure_request(ssl, request);
        SSL_shutdown(ssl->ssl);
        SSL_free(ssl->ssl);
        SSL_CTX_free(ssl->ctx);
        close(ssl->sfd);
        free(ssl);
    } else {
        int sfd = _insecure_connect(results);
        if (sfd < 0) {
            goto cleanup;
        }
        response = send_insecure_request(sfd, request);
        close(sfd);
    }

cleanup:
    freeaddrinfo(results);
    free_host_and_path(&hp);

    return response;
}

static char *cp_range_to_buffer(UriTextRangeA *rng, size_t max_len) {
    if (!rng || !rng->first) return NULL;
    size_t len = (size_t)(rng->afterLast - rng->first);
    if (len > max_len) {
        return NULL;
    } 

    char *buf = malloc(len + 1);
    if (!buf) {
        return NULL;
    }

    memcpy(buf, rng->first, len);
    buf[len] = '\0';
    return buf;
}

char *build_path(UriPathSegmentA *path_head) {
    if (!path_head) return NULL;

    size_t i = 0;
    int segment_length = 0;
    char *path_str = calloc(MAX_PATH_LEN, sizeof(char));

    UriPathSegmentA *segment;
    for (segment = path_head; segment != NULL; segment = segment->next) {
        path_str[i++] = '/';
        segment_length = segment->text.afterLast - segment->text.first; 
        memcpy(path_str + i, segment->text.first, segment_length);
        i += segment_length;
    }
    path_str[i] = '\0';

    return path_str;
}

static int parse_url(const char *url, host_and_path *hp) {
    if (!url) return 1;

    UriUriA uri;
    const char *errorPos; 
    
    if (uriParseSingleUriA(&uri, url, &errorPos) != URI_SUCCESS) {
        uriFreeUriMembersA(&uri);
        return 1;
    }
    if (!uri.hostText.first) {
        uriFreeUriMembersA(&uri);
        return 1;
    }
    hp->host = cp_range_to_buffer(&uri.hostText, MAX_DOMAIN_LEN);
    hp->path = build_path(uri.pathHead);
    hp->secure = strncmp(uri.scheme.first, "https", 5) != 0 ? 0 : 1;
    hp->port = cp_range_to_buffer(&uri.portText, 16);
    if (hp->port == NULL) {
        hp->port = hp->secure ? "443" : "80";
    }

    uriFreeUriMembersA(&uri);

    return 0;
}

void free_host_and_path(host_and_path *hp) {
    free(hp->host);
    free(hp->path);

    hp->host = NULL;
    hp->path = NULL;
}

void free_http_response(http_response *response) {
    if (!response) return;
    free(response->raw);
    free(response);
}