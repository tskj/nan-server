#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../src/request_handler.c"

int unit_test_handler(char* file, char* request) {

    int stdin = dup(0);
    close(0);
    
    int fd = open(file, O_RDONLY);
    if (-1 == fd) {
        printf("Couldn't open file: %s\n", file);
        return 1;
    }

    printf("%s\n", request);

    handle_request();

    printf("\n\n");

    close(fd);
    dup(stdin);

    return 0;
}

int main() {

	unit_test_handler("put-api.txt", "PUT /api/addressbook");
	unit_test_handler("get-api.txt", "GET /api/addressbook");
    unit_test_handler("post-api.txt", "POST /api/addressbook");
    unit_test_handler("get-request-index.txt", "GET /index.html");
    unit_test_handler("get-request-illegal.txt", "GET /lib");
    unit_test_handler("get-request.txt", "GET /not_found_file");
    unit_test_handler("get-request-pipe.txt", "GET User submitted content");

    printf("/png == /png: %d\n", path_is_match("/png", "/png"));
    printf("/png/ == /png: %d\n", path_is_match("/png/", "/png"));
    printf("/png/dk == /png: %d\n", path_is_match("/png/dk", "/png"));
    printf("/pngu != /png: %d\n", !path_is_match("/pngu", "/png"));

    return 0;
}
