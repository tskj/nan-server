#include <string.h>
#include <errno.h>

#include "request_handler.h"
#include "addressbook_handler.c"

const char const* illegal_paths[] = { "/bin"
                                    , "/lib"
                                    };


mime_t resolve_extension(char* filename) {
    
    int i = 0;
    while (filename[i] && filename[i] != '.') i++;

    if (0 == filename[i]) return NONE;
    i++;

    if (!strcmp(filename + i, "txt"))
        return PLAIN;
    if (!strcmp(filename + i, "html"))
        return HTML;
    if (!strcmp(filename + i, "css"))
        return CSS;
    if (!strcmp(filename + i, "png"))
        return PNG;
    if (!strcmp(filename + i, "xml"))
        return XML;
    if (!strcmp(filename + i, "xsl"))
        return XSL;
    if (!strcmp(filename + i, "dtd"))
        return DTD;
    if (!strcmp(filename + i, "js"))
        return JS;
 
    return UNKNOWN;
}

int path_is_match(char* req, const char* pattern) {
    int i = 0;
    while (1) {
        if (pattern[i] == 0)
            if (req[i] == '/' || req[i] == 0)
                return 1;
            else
                return 0;
        else if (req[i] != pattern[i])
            return 0;
        i++;
    }
}

header_t parse_request() {

    header_t header;

    int header_size = 512;
    char* buffer = malloc(header_size);
    int read_bytes = read(0, buffer, header_size);

    int consecutive_LFs = 0;
    int i = 0;

    do {

        while (i < read_bytes) {

            if (buffer[i] == '\n')
                consecutive_LFs++;
            else if (buffer[i] == '\r' || buffer[i] == ' ')
                consecutive_LFs = consecutive_LFs;
            else
                consecutive_LFs = 0;

            i++;

            if (consecutive_LFs == 2)
                break;
        }

        if (consecutive_LFs != 2) {
            read_bytes += read(0, buffer + i, header_size - read_bytes);
        }

        while (read_bytes == header_size) {
            header_size *= 2;
            char* new_buffer = malloc(header_size);
            memcpy(new_buffer, buffer, header_size/2);
            free(buffer);
            buffer = new_buffer;

            read_bytes += read(0, buffer + header_size/2, header_size/2);
        }

    } while (consecutive_LFs < 2);

    header.body = buffer + i;
    
    buffer[read_bytes] = '\0'; // Cannot possibly overflow due to while-loop

    i = 0;
    if        (!strncmp(buffer, "GET ",    GET_LENGTH))    {
        header.request = GET;
        i = GET_LENGTH;
    } else if (!strncmp(buffer, "PUT ",    PUT_LENGTH))    {
        header.request = PUT;
        i = PUT_LENGTH;
    } else if (!strncmp(buffer, "POST ",   POST_LENGTH))   {
        header.request = POST;
        i = POST_LENGTH;
    } else if (!strncmp(buffer, "HEAD ",   HEAD_LENGTH))   {
        header.request = HEAD;
        i = HEAD_LENGTH;
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

    header.type = resolve_extension(header.path);

    return header;
}

void send_header(status_code_t status_code, request_t req, mime_t content_type) {

    printf("HTTP/1.0 %d %s\n", status_code, string_status(status_code));

    printf("Content-Type: ");
    switch (content_type) {
        case PLAIN: printf("text/plain; charset=utf-8\n");
                    break;
        case HTML:  printf("text/html; charset=utf-8\n");
                    break;
        case CSS:   printf("text/css; charset=utf-8\n");
                    break;
        case PNG:   printf("image/png\n");
                    break;
        case XML:   printf("application/xml; charset=utf-8\n");
                    break;
        case XSL:   printf("text/xsl; charset=utf-8\n");
                    break;
        case JS:    printf("application/javascript; charset=utf-8\n");
                    break;
        case DTD:   printf("application/xml-dtd\n");
                    break;
        default:    printf("text/plain; charset=utf-8\n");
    }

    printf("Connection: close\n");
    printf("\n");

    fflush(stdout);

    if (req == HEAD) {
        exit(0);
    }
}

void send_file(char* path) {

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

    if (path_is_match(header.path, API_PATH)) {
        int offset = strlen(API_PATH) + 1;
        if (!strcmp(header.path, API_PATH)){
            send_header(OK, header.request, PLAIN);
            printf("Det einaste implementerte APIet er: %s\n", ADDRESSBOOK_API);
        }else if (!strncmp( header.path
                          , ADDRESSBOOK_API
                          , strlen(ADDRESSBOOK_API))) {

                                addressbook_handler(header);
        } else {
            send_header(NOT_IMPLEMENTED, header.request, PLAIN);
            printf("API: \"%s\" finnest ikkje\n", &header.path[offset]);
        }
        return;
    }

    if (header.request == ILLEGAL) {
        send_header(BAD_REQUEST, header.request, HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header.path);
        return;
    }

    if (header.type == UNKNOWN) {
        send_header(NOT_FOUND, header.request, HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header.path);
        return;
    }

    int i = 0;
    for (i = 0; i < sizeof(illegal_paths) / sizeof(illegal_paths[0]); i++) {
        if (path_is_match(header.path, illegal_paths[i])) {
            send_header(NOT_FOUND, header.request, HTML);
            header.path = NOT_FOUND_FILE;
            send_file(header.path);
            return;
        }
    }

    if (header.request != GET && header.request != HEAD) {
        send_header(METHOD_NOT_ALLOWED, header.request, HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header.path);
        return;
    }

    // Gonna have to serve a file...

    if (!strcmp(header.path, "/")) {
        header.path = "/index.html";
        header.type = HTML;
    }

    if (-1 == access(header.path, R_OK)) {
        send_header(NOT_FOUND, header.request, HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header.path);
        return;
    }

    send_header(OK, header.request, header.type);
    send_file(header.path);

    return;
}