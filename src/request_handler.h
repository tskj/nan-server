#ifndef REQUEST_HANDLER
#define REQUEST_HANDLER

#define DEFAULT_FILE "index.html"
#define NOT_FOUND_FILE "/lib/not-found.html"
#define API_PATH "/api"
#define ADDRESSBOOK_API "/api/addressbook"

#define GET_LENGTH 4
#define PUT_LENGTH 4
#define POST_LENGTH 5
#define HEAD_LENGTH 5
#define DELETE_LENGTH 7

typedef enum { GET
             , PUT
             , POST
             , HEAD
             , DELETE
             , ILLEGAL
             } request_t;

typedef enum { PLAIN
             , HTML
             , CSS
             , PNG
             , GIF
             , XML
             , XSL
             , DTD
             , JS
             , UNKNOWN
             , NONE
             } mime_t;

typedef struct {
    request_t  request;
    char*      path;
    mime_t     type;
    char*      host;
    char*      body;
} header_t;

typedef enum { OK                    = 200
             , CREATED               = 201
             , NO_CONTENT            = 204
             , SEE_OTHER             = 303
             , BAD_REQUEST           = 400
             , NOT_FOUND             = 404
             , METHOD_NOT_ALLOWED    = 405
             , INTERNAL_SERVER_ERROR = 500
             , NOT_IMPLEMENTED       = 501
             } status_code_t;

void send_header(status_code_t, header_t);

char* string_status(status_code_t code) {

    switch (code) {
        case OK:                    return "OK";
        case CREATED:               return "Created";
        case NO_CONTENT:            return "No Content";
        case SEE_OTHER:             return "See Other";
        case BAD_REQUEST:           return "Bad Request";
        case NOT_FOUND:             return "Not Found";
        case METHOD_NOT_ALLOWED:    return "Method Not Allowed";
        case INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case NOT_IMPLEMENTED:       return "Not Implemented";
    }

    return "Error, should never reach";
}

#endif