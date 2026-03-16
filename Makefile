TEST_IMPORT_FILE := "./config/channel_list.txt"

CFLAGS_MAIN := $(shell pkg-config --cflags openssl liburiparser)
LDFLAGS_MAIN := $(shell pkg-config --libs sqlite3 openssl liburiparser)

DEPS := $(shell find src -path src/termbox2 -prune -o -type f -name "*.[h|c]" -print) 
SOURCES := $(shell find src -path src/termbox2 -prune -o -type f -name "*.c" -print) 

RELEASE_OBJS := $(patsubst src/%.c, build/release/%.o, $(SOURCES))
DEBUG_OBJS := $(patsubst src/%.c, build/debug/%.o, $(SOURCES))

DEBUG_FLAGS := -Wall -Werror -g -O0
RELEASE_FLAGS := -Wall -O2

compile_release: $(RELEASE_OBJS)
compile_debug: $(DEBUG_OBJS)

build/release/%.o: src/%.c
	mkdir -p $(dir $@) 
	clang $(CFLAGS_MAIN) $(DEBUG_FLAGS) -c $< -o $@

build/debug/%.o: src/%.c
	mkdir -p $(dir $@) 
	clang $(CFLAGS_MAIN) $(RELEASE_FLAGS) -c $< -o $@

link_release: $(RELEASE_OBJS)  
	clang $(RELEASE_OBJS) -o main $(LDFLAGS_MAIN)

link_debug: $(DEBUG_OBJS) 
	clang $(DEBUG_OBJS) -o debug_main $(LDFLAGS_MAIN)

# For running tests

GTEST_CFLAGS := $(shell pkg-config --cflags gtest_main)
GTEST_LDFLAGS := $(shell pkg-config --libs gtest_main)

NO_MAIN_OBJS := $(shell find build/debug -name "main.o" -prune -o -type f -name "*.o" -print)

test: $(DEBUG_OBJS)
	clang++ -std=c++17 $(NO_MAIN_OBJS) -o run_tests test/*.cc $(GTEST_CFLAGS) $(LDFLAGS_MAIN) $(GTEST_LDFLAGS)
	./run_tests