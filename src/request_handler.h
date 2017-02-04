#ifndef REQUEST_HANDLER
#define REQUEST_HANDLER

#define NOT_FOUND_FILE "/lib/not-found.html"
#define API_PATH "/api"
#define ADDRESSBOOK_API "addressbook"

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
    char*      body;
} header_t;

void send_header(int, char*, request_t, mime_t);

#endif