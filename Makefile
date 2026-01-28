CLANG = clang -Wall -Werror -std=gnu11 -O0 -g

CFLAGS := $(shell pkg-config --cflags openssl)
LDFLAGS := $(shell pkg-config --libs openssl)

PARSER = src/parser

socket_test: src/socket_test.c
	${CLANG} ${CFLAGS} src/socket_test.c ${LDFLAGS} -o main

run_socket_test: socket_test
	./main

MAIN_DEPS =   src/*.c \
			  src/*.h \
			  src/channels_db/*.c \
			  src/channels_db/*.h \
			  src/parser/*.c \
			  src/parser/*.h \
			  src/ui/*.c \
			  src/ui/*.h \
			  src/ui/pages/*.c \
			  src/ui/pages/*.h

SOURCES = 	src/*.c \
			${PARSER}/*.c \
			src/ui/*.c \
			src/ui/pages/*.c \
			src/channels_db/*.c
CLANG_CMD = ${CLANG} ${CFLAGS} ${LDFLAGS} -o main ${SOURCES}
ASAN_CLANG_CMD = ${CLANG} ${CFLAGS} ${LDFLAGS} -fsanitize=address -o main ${SOURCES}

main: ${MAIN_DEPS}
	${CLANG_CMD}

asan_main: ${MAIN_DEPS}
	${ASAN_CLANG_CMD}

LDFLAGS := $(shell pkg-config --libs sqlite3)
db_testing: src/channels_db/*
	rm ripple.db || true
	${CLANG} ${LDFLAGS} -o db_testing src/channels_db/*.c
	./db_testing	

run_main: main
	rm ripple.db || true
	./main
run_main_asan: asan_main
	./main