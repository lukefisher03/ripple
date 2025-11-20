#include "xml_rss.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char *argv[]) {
    size_t size;
    char *rss = file_to_string("test/stack_overflow.xml", &size);
    struct rss_feed *feed = build_feed(rss, size);
    free(rss);
}