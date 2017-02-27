#include <string.h>
#include <sys/stat.h>

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
    if (!strcmp(filename + i, "gif"))
        return GIF;
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
    int diff_bytes = 0;

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
            diff_bytes = read_bytes;
            read_bytes += read(0, buffer + i, header_size - read_bytes);
            if (read_bytes <= diff_bytes) {
                // Connection closed prematurely
                exit(0);
            }
        }

        while (read_bytes == header_size) {
            header_size *= 2;
            char* new_buffer = malloc(header_size);
            memcpy(new_buffer, buffer, header_size/2);
            free(buffer);
            buffer = new_buffer;

            diff_bytes = read_bytes;
            read_bytes += read(0, buffer + header_size/2, header_size/2);
            if (read_bytes <= diff_bytes) {
                // Connection closed prematurely
                exit(0);
            }
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
    header.host = NULL;

    i++;
    while (i < read_bytes) {

        if (buffer[i] == 'H' && !strncmp("Host: ", &buffer[i], strlen("Host: "))) {
            header.host = &buffer[i + strlen("Host: ")];
            i += strlen("Host: ");
            while (i < read_bytes) {
                if (is_whitespace(buffer[i])) {
                    buffer[i] = '\0';
                    break;
                }
                i++;
            }
            break;
        }

        i++;
    }

    return header;
}

void send_header(status_code_t status_code, header_t h) {

    printf("HTTP/1.0 %d %s\n", status_code, string_status(status_code));

    if (status_code == SEE_OTHER) {
        printf("Location: http://%s%s\n", h.host, h.path);
    }

    printf("Content-Type: ");
    switch (h.type) {
        case PLAIN:     printf("text/plain; charset=utf-8\n");
                        break;
        case HTML:      printf("text/html; charset=utf-8\n");
                        break;
        case CSS:       printf("text/css; charset=utf-8\n");
                        break;
        case PNG:       printf("image/png\n");
                        break;
        case GIF:       printf("image/gif\n");
                        break;
        case XML:       printf("application/xml; charset=utf-8\n");
                        break;
        case XSL:       printf("text/xsl; charset=utf-8\n");
                        break;
        case JS:        printf("application/javascript; charset=utf-8\n");
                        break;
        case DTD:       printf("application/xml-dtd\n");
                        break;
        case NONE:      printf("text/plain; charset=utf-8\n");
                        break;
        case UNKNOWN:   printf("text/plain; charset=utf-8\n");
                        break;
    }

    printf("Connection: close\n");
    printf("\n");

    fflush(stdout);

    if (h.request == HEAD) {
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

    if (header.request == ILLEGAL) {
        header.type = HTML;
        send_header(BAD_REQUEST, header);
        return;
    }

    if (path_is_match(header.path, API_PATH)) {
        int offset = strlen(API_PATH) + 1;
        if (!strcmp(header.path, API_PATH)){
            header.type = PLAIN;
            send_header(OK, header);
            printf("Det einaste implementerte APIet er: %s\n", ADDRESSBOOK_API);
        }else if (path_is_match(header.path, ADDRESSBOOK_API)) {
            addressbook_handler(header);
        } else {
            header.type = PLAIN;
            send_header(NOT_IMPLEMENTED, header);
            printf("API: \"%s\" finnest ikkje\n", &header.path[offset]);
        }
        return;
    }

    if (header.type == UNKNOWN) {
        header.type = HTML;
        send_header(NOT_FOUND, header);
        header.path = NOT_FOUND_FILE;
        send_file(header.path);
        return;
    }

    int i = 0;
    for (i = 0; i < sizeof(illegal_paths) / sizeof(illegal_paths[0]); i++) {
        if (path_is_match(header.path, illegal_paths[i])) {
            header.type = HTML;
            send_header(NOT_FOUND, header);
            header.path = NOT_FOUND_FILE;
            send_file(header.path);
            return;
        }
    }

    if (header.request != GET && header.request != HEAD) {
        header.type = HTML;
        send_header(METHOD_NOT_ALLOWED, header);
        header.path = NOT_FOUND_FILE;
        send_file(header.path);
        return;
    }

    // Gonna have to serve a file...

    if (header.type == NONE && -1 != access(header.path, R_OK)) {
        struct stat s = {0};
        if (!stat(header.path, &s)) {
            if (S_ISDIR(s.st_mode)) {
                int path_length = strlen(header.path);
                char* fully_qualified_name = malloc(path_length + 1 + strlen(DEFAULT_FILE) + 1);
                strncpy(fully_qualified_name, header.path, path_length);
                int added_slash = 0;
                if (fully_qualified_name[path_length-1] != '/') {
                    fully_qualified_name[path_length] = '/';
                    added_slash = 1;
                }
                strncpy(fully_qualified_name + path_length + added_slash, DEFAULT_FILE, strlen(DEFAULT_FILE));
                fully_qualified_name[path_length + added_slash + strlen(DEFAULT_FILE)] = '\0';
                header.path = fully_qualified_name;
                header.type = resolve_extension(header.path);

                send_header(SEE_OTHER, header);
                exit(0);
            }
        } else {
            send_header(INTERNAL_SERVER_ERROR, header);
            exit(0);
        }
    }

    if (-1 == access(header.path, R_OK)) {
        header.type = HTML;
        send_header(NOT_FOUND, header);
        header.path = NOT_FOUND_FILE;
        send_file(header.path);
        return;
    }

    send_header(OK, header);
    send_file(header.path);

    return;
}