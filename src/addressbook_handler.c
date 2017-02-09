#include <stdlib.h>

#include "sqlite3.h"

#include "request_handler.h"
#include "xml_parser.c"
#include "xml_serializer.c"

#define DB_PATH "/api/addressbook.db"
#define BUFFER_SIZE 1024

void handle_get_request(header_t req) {

    char* id_to_get;
    int length;

    char* start_of_id = &req.path[strlen(ADDRESSBOOK_API)];
    char* invalid_token;
    if (*start_of_id == '/') start_of_id++;
    long int id = strtol(start_of_id, &invalid_token, 0);

    if ('\0' == *start_of_id) {
        id_to_get = "%";
    } else if ('\0' == *invalid_token) {
        int size = 128;
        id_to_get = malloc(size);
        int bytes_written = snprintf(id_to_get, size, "%ld", id);
        if (bytes_written >= size || bytes_written <= 0) {
            send_header(BAD_REQUEST, req.request, req.type);
            exit(0);
        }
    } else {
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    element_t* contacts = malloc(sizeof(element_t));

    contacts -> tag = "contacts";
    contacts -> attributes = NULL;
    contacts -> nodes = NULL;
    contacts -> text = NULL;

    sqlite3* db;

    sqlite3_stmt* sql_statement;
    
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc != SQLITE_OK) {
        send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
        exit(0);
    }

    char* SQL = "SELECT ID, Name, Tlf FROM Addressbook WHERE ID LIKE ? ORDER BY ID DESC;";

    rc = sqlite3_prepare_v2(db, SQL, -1, &sql_statement, NULL);

    if (rc != SQLITE_OK) {
            sqlite3_close(db);
            send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
            exit(0);
    }

    sqlite3_bind_text(sql_statement, 1, id_to_get, -1, NULL);

    node_t* p;
    while ((rc = sqlite3_step(sql_statement)) == SQLITE_ROW) {
        p = NULL;

        element_t* contact = malloc(sizeof(element_t));
        contact -> tag = "contact";
        contact -> attributes  = NULL;
        contact -> nodes = NULL;
        contact -> text = NULL;

        element_t* tlf = malloc(sizeof(element_t));
        tlf -> tag = "tlf";
        tlf -> attributes = NULL;
        tlf -> nodes = NULL;
        length = sqlite3_column_bytes(sql_statement, 2) + 1;
        tlf -> text = malloc(length);
        memcpy(tlf -> text, sqlite3_column_text(sql_statement, 2), length);

        p = contact -> nodes;

        contact -> nodes = malloc(sizeof(node_t));
        contact -> nodes -> element = tlf;
        contact -> nodes -> sibling = p;

        element_t* name = malloc(sizeof(element_t));
        name -> tag = "name";
        name -> attributes = NULL;
        name -> nodes = NULL;
        length = sqlite3_column_bytes(sql_statement, 1) + 1;
        name -> text = malloc(length);
        memcpy(name -> text, sqlite3_column_text(sql_statement, 1), length);

        p = contact -> nodes;

        contact -> nodes = malloc(sizeof(node_t));
        contact -> nodes -> element = name;
        contact -> nodes -> sibling = p;

        element_t* id = malloc(sizeof(element_t));
        id -> tag = "id";
        id -> attributes = NULL;
        id -> nodes = NULL;
        id -> text = malloc(BUFFER_SIZE);
        snprintf(id -> text, BUFFER_SIZE, "%d", sqlite3_column_int(sql_statement, 0));

        p = contact -> nodes;

        contact -> nodes = malloc(sizeof(node_t));
        contact -> nodes -> element = id;
        contact -> nodes -> sibling = p;

        p = contacts -> nodes;

        contacts -> nodes = malloc(sizeof(node_t));
        contacts -> nodes -> element = contact;
        contacts -> nodes -> sibling = p;
    }

    sqlite3_finalize(sql_statement);
    sqlite3_close(db);

    if (rc != SQLITE_DONE) {
        send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
        exit(0);
    }

    if (NULL == contacts -> nodes) {
        send_header(NO_CONTENT, req.request, req.type);
        exit(0);
    }

    send_header(OK, req.request, XML);
    printf("%s", serialize_xml(contacts));
    fflush(stdout);
}

void addressbook_handler(header_t req) {

    switch (req.request) {

        case GET:       handle_get_request(req);
                        break;
        case HEAD:      handle_get_request(req);
                        break;
/*        case POST:      handle_post_request(req);
                        break;
        case PUT:       handle_put_request(req);
                        break;
        case DELETE:    handle_delete_request(req);
                        break;
        case ILLEGAL:   send_header(METHOD_NOT_ALLOWED, req.request, req.type);
                        exit(0);
                        */
        default:        exit(0);
    }
}