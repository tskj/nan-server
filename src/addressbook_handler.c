#include "request_handler.h"
#include "xml_parser.c"
#include "xml_serializer.c"

void addressbook_handler(header_t req) {

    element_t* root = parse_xml(req.body);

    if (!root) {
        send_header(404, "BAD", req.request, PLAIN);
        printf("Failed to parse\n");
        exit(0);
    }
    send_header(201, "Created", req.request, XML);
    printf("%s\n", serialize_xml(root));
    fflush(0);
}