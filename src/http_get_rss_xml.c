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

struct  ssl_connection {
    SSL_CTX  *ctx; // SSL context
    SSL      *ssl; // SSL object
    int       sfd; // Attached file descriptor
};

static struct ssl_connection *_secure_connect(struct addrinfo *results, const char *host) {
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
static int _insecure_connect(struct addrinfo *results) {
    struct addrinfo *rp;
    int sfd;

    for (rp = results; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            log_debug("Found connection!");
            return sfd;
        }
        close(sfd);
    }
    log_debug("Failed to find connection");
    return -1;
}

static http_response *send_insecure_request(int sfd, char *request) {
    size_t bytes_read = 0;
    size_t cap = 64000;

    char *raw_response = malloc(cap);
    if (!raw_response) {
        log_debug("Allocating buffer for response failed: %s", strerror(errno));
        return NULL;
    }

    int ret_code;
    if ((ret_code = write(sfd, request, strlen(request) + 1)) <= 0) {
        log_debug("Failed to send request:\n%s\nError message: %s", request, strerror(errno));
        return NULL;
    }

    ssize_t bytes;
    while ((bytes = read(sfd, raw_response + bytes_read, cap - bytes_read - 1)) > 0) {
        if (bytes <= 0) {
            log_debug("Failure while reading response! %s", strerror(errno));
            return NULL;
        }
        bytes_read += bytes;
        if (bytes_read >= MAX_REQUEST_RESPONSE_SIZE) {
            log_debug("HTTP response is too big for the buffer. Max buffer size: %lu bytes", MAX_REQUEST_RESPONSE_SIZE);
            free(raw_response);
            return NULL;
        }

        if (bytes_read >= cap - 1) {
            cap *= 2;
            char *tmp = realloc(raw_response, cap);

            if (!tmp) {
                log_debug("Buffer expansion with realloc failed: %s", strerror(errno));
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

static http_response *send_secure_request(struct ssl_connection *ssl_items, char *request) {
    size_t bytes_read = 0;
    size_t cap = 64000;

    char *raw_response = malloc(cap);

    int ret_code;
    if ((ret_code = SSL_write(ssl_items->ssl, request, strlen(request) + 1)) <= 0) {
        int error_code = SSL_get_error(ssl_items->ssl, ret_code);
        log_debug("Could not send get request! %s\n", ERR_error_string(error_code, NULL));
        return NULL;
    }

    ssize_t bytes;
    while((bytes = SSL_read(ssl_items->ssl, raw_response + bytes_read, cap - bytes_read - 1)) > 0) {
        if (bytes <= 0) {
            int error_code = SSL_get_error(ssl_items->ssl, bytes);
            log_debug("Encountered an error when reading response, %s\n", ERR_error_string(error_code, NULL));
            return NULL;
        }
        bytes_read += bytes;
        if (bytes_read >= MAX_REQUEST_RESPONSE_SIZE) {
            free(raw_response);
            log_debug("Request response too large to process. Response size capped at %lu", MAX_REQUEST_RESPONSE_SIZE);
            log_debug("Associated request: %s", request);
            return NULL;
        }
        if (bytes_read >= cap - 1) {
            cap *= 2; // Amortized O(1) append
            char *tmp = realloc(raw_response, cap);

            if (!tmp) {
                log_debug("Memory error, failed to allocate new space for response.\n");
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
    host_and_path hp;
    if (parse_url(url, &hp) != 0) {
        log_debug("Failed to parse url, skipping");
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
    log_debug("%s %s %s %d", hp.host, hp.path, hp.port, hp.secure);
    if (s != 0) {
        log_debug("getaddrinfo error: %d %s\n", s, gai_strerror(s));
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
        log_debug("Failed to write request. Either it was too large or an error occurred.");
        log_debug("Associated request: %s", request);
        return NULL;
    }
    
    http_response *response = NULL;

    if (hp.secure) {
        struct ssl_connection *ssl = _secure_connect(results, hp.host);
        response = send_secure_request(ssl, request);
        SSL_shutdown(ssl->ssl);
        SSL_free(ssl->ssl);
        SSL_CTX_free(ssl->ctx);
        close(ssl->sfd);
        free(ssl);
    } else {
        log_debug("Attempting to make connection");
        int sfd = _insecure_connect(results);
        log_debug("File descriptor: %d", sfd);
        response = send_insecure_request(sfd, request);
        log_debug("Finished request! %s", response->body);
        close(sfd);
    }

    freeaddrinfo(results);
    
    if (!response) {
        log_debug("Failed to get RSS XML for feed url: %s", hp.host);
    }
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
        log_debug("Failed to allocate space for url host");
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

    log_debug("PATH: %s", path_str);
    return path_str;
}

static int parse_url(const char *url, host_and_path *hp) {
    if (!url) return 1;

    UriUriA uri;
    const char *errorPos; 
    
    if (uriParseSingleUriA(&uri, url, &errorPos) != URI_SUCCESS) {
        log_debug("Failed to parse uri: %s", url);
        log_debug("Error: %s", errorPos);
        uriFreeUriMembersA(&uri);
        return 1;
    }
    if (!uri.hostText.first) {
        log_debug("Couldn't parse url, missing scheme (https, http) - %s", url);
        uriFreeUriMembersA(&uri);
        return 1;
    }
    hp->host = cp_range_to_buffer(&uri.hostText, MAX_DOMAIN_LEN);
    hp->path = build_path(uri.pathHead);
    hp->secure = strncmp(uri.scheme.first, "https", 5) != 0 ? 0 : 1;
    hp->port = cp_range_to_buffer(&uri.portText, 16);
    log_debug("PORT: %s", hp->port);
    if (hp->port == NULL) {
        log_debug("Setting port!");
        hp->port = hp->secure ? "443" : "80";
    }

    log_debug("%s %s", hp->host, hp->port); 
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