TEST_IMPORT_FILE = "channel_list.txt"

CLANG_DEBUG = clang -Wall -Werror -std=gnu11 -O0 -g -fsanitize=address
CLANG_PROD = clang -Wall -Werror -std=gnu11 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE 

CFLAGS_MAIN := $(shell pkg-config --cflags openssl liburiparser)
LDFLAGS_MAIN := $(shell pkg-config --libs sqlite3 openssl liburiparser)

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
			src/parser/*.c \
			src/ui/*.c \
			src/ui/pages/*.c \
			src/channels_db/*.c

MAIN_BUILD = -o main ${SOURCES} ${CFLAGS_MAIN} ${LDFLAGS_MAIN} 

main: ${MAIN_DEPS}
	${CLANG_PROD} ${MAIN_BUILD}

debug_main: ${MAIN_DEPS}
	${CLANG_DEBUG} ${MAIN_BUILD}

test_run_main: main
	./main $(TEST_IMPORT_FILE)

test_debug_main: debug_main
	./main $(TEST_IMPORT_FILE)