#include "request_handler.h"
#include "xml_parser.c"

void addressbook_handler(header_t req) {

    element_t* root = parse_xml(req.body);

    if (!root) {
        send_header(404, "BAD", req.request, PLAIN);
        printf("Failed to parse\n");
        exit(0);
    }
    send_header(200, "OK", req.request, req.type);
    printf("Rotnamn: %s\n", root -> nodes -> element -> text);
}