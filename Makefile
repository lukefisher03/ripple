CLANG = clang -Wall -Werror -std=gnu11 -O0 -g

CFLAGS_MAIN := $(shell pkg-config --cflags openssl)
CFLAGS_SOCK_TEST := $(shell pkg-config --cflags openssl liburiparser)
LDFLAGS_SOCK_TEST := $(shell pkg-config --libs openssl liburiparser)
LDFLAGS_MAIN := $(shell pkg-config --libs sqlite3)

PARSER = src/parser

http_get_rss_xml: src/http_get_rss_xml.c src/http_get_rss_xml.h
	${CLANG} ${CFLAGS_SOCK_TEST} src/http_get_rss_xml.c src/logger.c ${LDFLAGS_SOCK_TEST} -o main

run_http_get_rss_xml: http_get_rss_xml 
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
CLANG_CMD = ${CLANG} ${CFLAGS_MAIN} ${LDFLAGS_MAIN} -o main ${SOURCES}
ASAN_CLANG_CMD = ${CLANG} ${CFLAGS_MAIN} ${LDFLAGS_MAIN} -fsanitize=address -o main ${SOURCES}

main: ${MAIN_DEPS}
	${CLANG_CMD}

asan_main: ${MAIN_DEPS}
	${ASAN_CLANG_CMD}

db_testing: src/channels_db/*
	rm ripple.db || true
	${CLANG} ${LDFLAGS_MAIN} -o db_testing src/channels_db/*.c
	./db_testing	

run_main: main
	rm ripple.db || true
	./main
run_main_asan: asan_main
	./main