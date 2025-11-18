CLANG = clang -std=gnu11 -O0 -g

CFLAGS += $(shell pkg-config --cflags openssl)
LDFLAGS += $(shell pkg-config --libs libssl libcrypto)

socket_test: socket_test.c
	${CLANG} ${CFLAGS} socket_test.c ${LDFLAGS} -o main

run_socket_test: socket_test
	./main

xml_tree: xml_tree.c utils.c utils.h rss_xml_tag.h rss_xml_tag.c stack.h stack.c dynamic_string.h dynamic_string.c
	${CLANG} -o main xml_tree.c utils.c rss_xml_tag.c stack.c dynamic_string.c

run_xml_tree: xml_tree
	./main