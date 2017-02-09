#include <stdlib.h>

#include "sqlite3.h"

#include "request_handler.h"
#include "xml_parser.c"
#include "xml_serializer.c"

#define DB_PATH "/api/addressbook.db"
#define VALREQ "/bin/xmlvalreq"
#define VALRES "/bin/xmlvalres"

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
        if (1 == length) {
            name -> text = NULL;
        } else {
            name -> text = malloc(length);
            memcpy(name -> text, sqlite3_column_text(sql_statement, 1), length);
        }

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

void handle_post_request(header_t req) {

    // Only want to accept
    // /api/addressbook or /api/addressbook/
    if (strlen(req.path) > strlen(ADDRESSBOOK_API) + 1) {
        send_header(NOT_IMPLEMENTED, req.request, PLAIN);
        exit(0);
    }

    int validation_req = open(VALREQ, O_WRONLY | O_NONBLOCK);
    if (-1 == validation_req) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }

    int i = 0;
    while (req.body[i]) {
        int written_bytes = write(validation_req, &req.body[i], 1);
        if (1 != written_bytes) {
            send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
            exit(0);
        }
        i++;
    }
    close(validation_req);

    char* happy_response = "- - valid";

    int validation_res = open(VALRES, O_RDONLY);
    if (-1 == validation_res) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }

    char buffer[strlen(happy_response)];
    int read_bytes = read(validation_res, buffer, sizeof(buffer));
    if (-1 == read_bytes) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }
    close(validation_res);

    if (strncmp(happy_response, buffer, sizeof(buffer))) {
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    // -------------------------------------- //
    // We now know we have received valid XML //
    // data with a valid HTTP (POST) request. //
    // -------------------------------------- //

    element_t* contacts = parse_xml(req.body);
    if (!contacts) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }

    node_t* contact = contacts -> nodes;
    while (contact) {

        sqlite3* db;
        sqlite3_stmt* sql_statement;

        int rc = sqlite3_open(DB_PATH, &db);

        if (rc != SQLITE_OK) {
            send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
            exit(0);
        }

        char* SQL = "INSERT INTO Addressbook(ID, Name, Tlf) VALUES (?,?,?);";

        rc = sqlite3_prepare_v2(db, SQL, -1, &sql_statement, NULL);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(sql_statement);
            sqlite3_close(db);
            send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
            exit(0);
        }

        char* invalid_char;
        char* id_tag = find_text_by_tag(contact -> element, "id");
        if (!id_tag) {
            sqlite3_finalize(sql_statement);
            sqlite3_close(db);
            send_header(BAD_REQUEST, req.request, req.type);
            exit(0);
        }

        int id = (int) strtol(id_tag, &invalid_char, 0);
        if ('\0' != *invalid_char) {
            sqlite3_finalize(sql_statement);
            sqlite3_close(db);
            send_header(BAD_REQUEST, req.request, req.type);
            exit(0);
        }

        sqlite3_bind_int(sql_statement, 1, id);

        sqlite3_bind_text(sql_statement, 2, find_text_by_tag(contact -> element, "name"), -1, NULL);

        char* tlf = find_text_by_tag(contact -> element, "tlf");
        if (!tlf) {
            sqlite3_finalize(sql_statement);
            sqlite3_close(db);
            send_header(BAD_REQUEST, req.request, req.type);
            exit(0);
        }

        sqlite3_bind_text(sql_statement, 3, tlf, -1, NULL);

        rc = sqlite3_step(sql_statement);
        if (rc == SQLITE_CONSTRAINT) {
            sqlite3_finalize(sql_statement);
            sqlite3_close(db);
            send_header(BAD_REQUEST, req.request, req.type);
            exit(0);
        }

        if (rc != SQLITE_DONE) {
            sqlite3_finalize(sql_statement);
            sqlite3_close(db);
            send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
            exit(0);
        }

        sqlite3_finalize(sql_statement);
        sqlite3_close(db);

        contact = contact -> sibling;
    }

    send_header(CREATED, req.request, req.type);
}

void handle_put_request(header_t req) {

    char* invalid_token;
    char* start_of_id = &req.path[strlen(ADDRESSBOOK_API)];
    if (*start_of_id != '/') {
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    start_of_id++;

    int id_to_update = (int) strtol(start_of_id, &invalid_token, 0);

    if ('\0' == *start_of_id) {
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    } else if ('\0' != *invalid_token) {
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    int validation_req = open(VALREQ, O_WRONLY | O_NONBLOCK);
    if (-1 == validation_req) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }

    int i = 0;
    while (req.body[i]) {
        int written_bytes = write(validation_req, &req.body[i], 1);
        if (1 != written_bytes) {
            send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
            exit(0);
        }
        i++;
    }
    close(validation_req);

    char* happy_response = "- - valid";

    int validation_res = open(VALRES, O_RDONLY);
    if (-1 == validation_res) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }

    char buffer[strlen(happy_response)];
    int read_bytes = read(validation_res, buffer, sizeof(buffer));
    if (-1 == read_bytes) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }
    close(validation_res);

    if (strncmp(happy_response, buffer, sizeof(buffer))) {
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    element_t* contacts = parse_xml(req.body);
    if (!contacts) {
        send_header(INTERNAL_SERVER_ERROR, req.request, PLAIN);
        exit(0);
    }

    element_t* contact = contacts -> nodes -> element;

    // Make sure there is only one node to be updated
    if (contacts -> nodes -> sibling != NULL) {
        send_header(BAD_REQUEST, req.request, PLAIN);
        exit(0);
    }

    sqlite3* db;
    sqlite3_stmt* sql_statement;

    int rc = sqlite3_open(DB_PATH, &db);

    if (rc != SQLITE_OK) {
        send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
        exit(0);
    }

    char* SQL = "UPDATE Addressbook SET ID = ?, Name = ?, Tlf = ? WHERE ID = ?;";

    rc = sqlite3_prepare_v2(db, SQL, -1, &sql_statement, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(sql_statement);
        sqlite3_close(db);
        send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
        exit(0);
    }

    char* invalid_char;
    char* id_tag = find_text_by_tag(contact, "id");
    if (!id_tag) {
        sqlite3_finalize(sql_statement);
        sqlite3_close(db);
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    int id = (int) strtol(id_tag, &invalid_char, 0);
    if ('\0' != *invalid_char) {
        sqlite3_finalize(sql_statement);
        sqlite3_close(db);
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    sqlite3_bind_int(sql_statement, 1, id);

    sqlite3_bind_text(sql_statement, 2, find_text_by_tag(contact, "name"), -1, NULL);

    char* tlf = find_text_by_tag(contact, "tlf");
    if (!tlf) {
        sqlite3_finalize(sql_statement);
        sqlite3_close(db);
        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    sqlite3_bind_text(sql_statement, 3, tlf, -1, NULL);

    sqlite3_bind_int(sql_statement, 4, id_to_update);

    rc = sqlite3_step(sql_statement);

    // ID likely didn't exist
    if (sqlite3_changes(db) != 1) {
        sqlite3_finalize(sql_statement);
        sqlite3_close(db);

        send_header(BAD_REQUEST, req.request, req.type);
        exit(0);
    }

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(sql_statement);
        sqlite3_close(db);
        send_header(INTERNAL_SERVER_ERROR, req.request, req.type);
        exit(0);
    }

    sqlite3_finalize(sql_statement);
    sqlite3_close(db);

    send_header(OK, req.request, req.type);
}

void addressbook_handler(header_t req) {

    switch (req.request) {

        case GET:       handle_get_request(req);
                        break;
        case HEAD:      handle_get_request(req);
                        break;
        case POST:      handle_post_request(req);
                        break;
        case PUT:       handle_put_request(req);
                        break;
                        /**
        case DELETE:    handle_delete_request(req);
                        break;
        case ILLEGAL:   send_header(METHOD_NOT_ALLOWED, req.request, req.type);
                        exit(0);
                        */
        default:        exit(0);
    }
}