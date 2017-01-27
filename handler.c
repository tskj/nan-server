#include <string.h>

#define NOT_FOUND_FILE "/lib/not-found.html"

#define GET_LENGTH 4
#define PUT_LENGTH 4
#define POST_LENGTH 5
#define DELETE_LENGTH 7

typedef enum { GET
             , PUT
             , POST
             , DELETE
             , ILLEGAL
             } request_t;

typedef struct {
    request_t  request;
    char*      path;
    char*      body;
} header_t;

typedef enum { PLAIN
             , HTML
             } mime_t;

header_t parse_request() {

    header_t header;

    int header_size = 512;
    char* buffer = malloc(header_size);
    int read_bytes = read(0, buffer, header_size);

    while (header_size <= read_bytes) {
        header_size *= 2;
        char* new_buffer = malloc(header_size);
        memcpy(new_buffer, buffer, header_size/2);
        free(buffer);
        buffer = new_buffer;

        read_bytes += read(0, buffer + header_size/2, header_size/2);
    }
    
    buffer[read_bytes] = '\0'; // Cannot possibly overflow due to while-loop

    int i = 0;
    if        (!strncmp(buffer, "GET ",    GET_LENGTH))    {
        header.request = GET;
        i = GET_LENGTH;
    } else if (!strncmp(buffer, "PUT ",    PUT_LENGTH))    {
        header.request = PUT;
        i = PUT_LENGTH;
    } else if (!strncmp(buffer, "POST ",   POST_LENGTH))   {
        header.request = POST;
        i = POST_LENGTH;
    } else if (!strncmp(buffer, "DELETE ", DELETE_LENGTH)) {
        header.request = DELETE;
        i = DELETE_LENGTH;
    } else {
        header.request = ILLEGAL;
        return header;
    }

    header.path = buffer + i;

    while (i < read_bytes && buffer[i] != ' ') i++;

    if (i == read_bytes) {
        header.request = ILLEGAL;
        return header;
    }

    buffer[i] = '\0';

    int consecutive_LFs = 0;
    while (i < read_bytes) {
        if (buffer[i] == '\n') {
            consecutive_LFs++;
        } else if (buffer[i] == ' ' || buffer[i] == '\r') {
            consecutive_LFs = consecutive_LFs; // noOp
        } else {
            consecutive_LFs = 0;
        }

        i++;

        if (consecutive_LFs == 2) {
            break;
        }
    }

    header.body = buffer + i;

    return header;
}

void send_header(int status_code, char* status, mime_t content_type) {

    printf("HTTP/1.0 %d %s\n", status_code, status);

    printf("Content-Type: ");
    switch (content_type) {
        case PLAIN: printf("text/plain; charset=utf-8\n");
                    break;
        case HTML:  printf("text/html\n");
                    break;
    }

    printf("Connection: close\n");
    printf("\n");

    fflush(stdout);
}

void send_file(const char* path) {

    char* buffer[4096];
    int fd = open(path, O_RDONLY);

    int written_bytes;
    int read_bytes = read(fd, buffer, 4069);
    while (read_bytes) {
        written_bytes = write(1, buffer, read_bytes);
        if (-1 == written_bytes) exit(1);
        read_bytes = read(fd, buffer, 4096);
    }
    close(fd);
}

void handle_request() {

    header_t header = parse_request();

    if (header.request == ILLEGAL) {
        send_header(404, "Not Found", HTML);
        send_file(NOT_FOUND_FILE);
        return;
    }

    // Simple test:
    send_header(200, "OK", PLAIN);
    printf("Yalla");
}