CLANG = clang -Wall -Werror -std=gnu11 -O0 -g

CFLAGS := $(shell pkg-config --cflags openssl)
LDFLAGS := $(shell pkg-config --libs libssl libcrypto)

PARSER = src/parser

socket_test: src/socket_test.c
	${CLANG} ${CFLAGS} src/socket_test.c ${LDFLAGS} -o main

run_socket_test: socket_test
	./main

CFLAGS := $(shell pkg-config --cflags readline)
LDFLAGS := $(shell pkg-config --libs readline)

MAIN_DEPS =   src/main.c \
			  src/logger.c src/logger.h \
			  src/utils.c \
			  src/utils.h \
			  src/list.h src/list.c \
			  src/ui/*.h src/ui/*.c \
			  src/ui/pages/*.h src/ui/pages/*.c \
			  ${PARSER}/xml_rss.h ${PARSER}/xml_rss.c \
			  ${PARSER}/node.h ${PARSER}/node.c \
			  src/arena.h src/arena.c 

CLANG_CMD = ${CLANG} -o main src/main.c src/logger.c src/utils.c ${PARSER}/*.c src/list.c src/ui/*.c src/ui/pages/*.c src/arena.c
ASAN_CLANG_CMD = ${CLANG} -fsanitize=address -o main src/main.c src/logger.c src/utils.c ${PARSER}/*.c src/list.c src/ui/*.c src/ui/pages/*.c src/arena.c

main: ${MAIN_DEPS}
	${CLANG_CMD}

asan_main: ${MAIN_DEPS}
	${ASAN_CLANG_CMD}

run_main: main
	./main
run_main_asan: asan_main
	./main