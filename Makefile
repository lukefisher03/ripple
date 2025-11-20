CLANG = clang -std=gnu11 -O0 -g

CFLAGS += $(shell pkg-config --cflags openssl)
LDFLAGS += $(shell pkg-config --libs libssl libcrypto)

socket_test: socket_test.c
	${CLANG} ${CFLAGS} socket_test.c ${LDFLAGS} -o main

run_socket_test: socket_test
	./main

main: main.c utils.c utils.h xml_rss.h xml_rss.c stack.h stack.c dynamic_string.h dynamic_string.c
	${CLANG} -o main main.c utils.c xml_rss.c stack.c dynamic_string.c

run_main: main
	./main