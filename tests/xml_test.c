#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "../src/xml_parser.c"

void print_a(attribute_t* at) {
    if (!at) return;
    printf("%s=\"%s\" ", at -> key, at -> value);
    print_a(at -> sibling);
}

void print_e(element_t* xml, int tab_level) {

    if (!xml) return;

    int i = 0;
    while (i++ < tab_level) printf("\t");

    printf("%s: ", xml -> tag);
    print_a(xml -> attributes);
    printf("\n");
    i = 0;
    while (i++ < tab_level+1) printf("\t");
    printf("%s\n", xml -> text);
    node_t* n = xml -> nodes;
    while (n) {
        print_e(n -> element, tab_level + 1);
        n = xml -> nodes -> sibling;
    }
}

int main() {

    int fd = open("example.xml", O_RDONLY);

    char buffer[4096];
    int read_bytes = read(fd, buffer, 4096);
    buffer[read_bytes] = '\0';

    element_t* AST = parse_xml(buffer);
    print_e(AST, 0);
}
