#include <string.h>
#include <errno.h>

#define NOT_FOUND_FILE "/lib/not-found.html"
#define API_PATH "/api"

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

    int header_size = 25;
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

    header.body = buffer + i;

    header.type = resolve_extension(header.path);

    return header;
}

void send_header(int status_code, char* status, mime_t content_type) {

    printf("HTTP/1.0 %d %s\n", status_code, status);

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
        default:    printf("UUh, unknown file, shouldn't have gotten here\n'");
    }

    printf("Connection: close\n");
    printf("\n");

    fflush(stdout);
}

void send_file(header_t header) {

    if (header.request == HEAD)
        return;

    char* path = header.path;

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
        send_header(200, "OK", PLAIN);
        execl("/bin/cat", "cat", "example.json", NULL);
        printf("%s\n", strerror(errno));
        return;
    }

    if (header.request == ILLEGAL) {
        send_header(404, "Not Found", HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header);
        return;
    }

    if (header.type == UNKNOWN) {
        send_header(404, "Not Found", HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header);
        return;
    }

    int i = 0;
    for (i = 0; i < sizeof(illegal_paths) / sizeof(illegal_paths[0]); i++) {
        if (path_is_match(header.path, illegal_paths[i])) {
            send_header(404, "Not Found", HTML);
            header.path = NOT_FOUND_FILE;
            send_file(header);
            return;
        }
    }

    if (header.request != GET && header.request != HEAD) {
        send_header(404, "Not Found", HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header);
        return;
    }

    // Gonna have to serve a file...

    if (!strcmp(header.path, "/")) {
        header.path = "/index.html";
        header.type = HTML;
    }

    if (-1 == access(header.path, R_OK)) {
        send_header(404, "Not Found", HTML);
        header.path = NOT_FOUND_FILE;
        send_file(header);
        return;
    }

    send_header(200, "OK", header.type);
    send_file(header);

    return;
}