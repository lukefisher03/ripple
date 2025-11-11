#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

struct SSL_CONNECTION {
    SSL_CTX  *ctx; // SSL context
    SSL      *ssl; // SSL object
    int       sfd; // Attached file descriptor
};

struct SSL_CONNECTION *_ssl_connect(struct addrinfo *results, const char *host) {
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
        fprintf(stderr, "SSL_CTX_new failed!\n");
        close(socket_file_descriptor);
        return NULL;
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket_file_descriptor); // Bind the ssl socket object to the socket fd.
    SSL_set_tlsext_host_name(ssl, host); // Some servers won't accept connections without this.
   
    int e;
    if ((e = SSL_connect(ssl)) <= 0) {
        e = SSL_get_error(ssl, e);
        printf("TLS Handshake failed! %d\n", e);
        printf("%s\n", ERR_error_string(e, NULL));
        close(socket_file_descriptor);
        return NULL;
    }

    struct SSL_CONNECTION *ssl_items = (struct SSL_CONNECTION *)malloc(sizeof(struct SSL_CONNECTION));
    
    if (!ssl_items) {
        printf("Failed to allocate SSL connection struct!\n");
        return NULL;
    }
    
    ssl_items->ctx = ctx;
    ssl_items->ssl = ssl;
    ssl_items->sfd = socket_file_descriptor;

    return ssl_items;
}

char * request_rss_xml(struct SSL_CONNECTION *ssl_items, const char * host, const char * path) {
    size_t bytes_read = 0;
    size_t cap = 64000;

    char *response = (char *)malloc(cap);
    char request[1024];

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
        printf("Could not send get request! %s\n", ERR_error_string(error_code, NULL));
        return NULL;
    }

    ssize_t bytes;
    while((bytes = SSL_read(ssl_items->ssl, response + bytes_read, cap - bytes_read - 1)) > 0) {
        if (bytes <= 0) {
            int error_code = SSL_get_error(ssl_items->ssl, bytes);
            printf("Encountered an error when reading response, %s\n", ERR_error_string(error_code, NULL));
            return NULL;
        }
        bytes_read += bytes;
        if (bytes_read >= cap - 1) {
            cap *= 2;
            char *tmp = (char *)realloc(response, cap);
            if (tmp) {
                response = tmp;
            } else {
                printf("Memory error, failed to allocate new space for response.\n");
                free(response);
                return NULL;
            }
        }
    }

    response[bytes_read] = '\0';
    return response;
}

char * get_feed_xml(const char *host, const char *path) {
    int                 sfd, s; // Socket file descriptor and status variable for getaddrinfo.
    struct addrinfo     hints; // Narrow down results of getaddrinfo.
    struct addrinfo     *results, *rp;  // Pointer to head of linked list in which getaddrinfo will
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
    s = getaddrinfo(host, "https", &hints, &results);

    if (s != 0) {
        fprintf(stderr, "getaddrinfo error: %d %s\n", s, gai_strerror(s));
        return NULL;
    }

    struct SSL_CONNECTION *ssl = _ssl_connect(results, host);
    freeaddrinfo(results);
    char * rss = request_rss_xml(ssl, host, path);
   
    SSL_shutdown(ssl->ssl);
    SSL_free(ssl->ssl);
    SSL_CTX_free(ssl->ctx);
    close(ssl->sfd);
    free(ssl);
    return rss;
}

// Get RSS feeds, written in C Sockets
void print_returned_addresses(struct addrinfo *results, FILE *stream) {
    if (stream == NULL) 
        stream = stdout;

    struct addrinfo *rp = results;
    size_t i = 0;
    
    if (rp == NULL) {
        fprintf(stream, "Linked list is empty\n");
        return;
    }

    // Iterate through all the returned results
    while (rp != NULL) {
        i++;
        char host[1024];
        void *addr;
        fprintf(stream, "Result %lu\n", i);
        if (rp->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
            addr = &(ipv4->sin_addr);
            fprintf(stream, "Protocol: IPv4\n");
        } else if (rp->ai_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)rp->ai_addr;
            addr = &(ipv6->sin6_addr);
            fprintf(stream, "Protocol: IPv6\n");
        }
        fprintf(stream, "Address: %s\n", inet_ntop(rp->ai_family, addr, host, sizeof(host)));
        fprintf(stream, "---------------------------------------------------\n");
        rp = rp->ai_next;
    }
}


int main(int argc, char const *argv[]) {
    const char * xml = get_feed_xml("news.ycombinator.com", "/rss");
    printf("%s\n", xml);
    return 0;
}


