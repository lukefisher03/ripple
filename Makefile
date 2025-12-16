CLANG = clang -std=gnu11 -O0 -g

CFLAGS := $(shell pkg-config --cflags openssl)
LDFLAGS := $(shell pkg-config --libs libssl libcrypto)

PARSER = src/parser

socket_test: src/socket_test.c
	${CLANG} ${CFLAGS} src/socket_test.c ${LDFLAGS} -o main

run_socket_test: socket_test
	./main

CFLAGS := $(shell pkg-config --cflags readline)
LDFLAGS := $(shell pkg-config --libs readline)

main: src/main.c \
	  src/utils.c \
	  src/utils.h \
	  src/list.h src/list.c \
	  src/ui/ui.h src/ui/ui.c \
	  ${PARSER}/xml_rss.h ${PARSER}/xml_rss.c \
	  ${PARSER}/node.h ${PARSER}/node.c \
	  src/arena.h src/arena.c
	  
	${CLANG} ${CFLAGS} ${LDFLAGS} -o main src/main.c src/utils.c ${PARSER}/xml_rss.c src/list.c ${PARSER}/node.c src/ui/ui.c src/arena.c

run_main: main
	./main