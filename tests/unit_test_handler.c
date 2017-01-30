#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../handler.c"

int main() {

    close(0);
    if (-1 == open("debug/get-request-index.txt", O_RDONLY))
        return 1;

    handle_request();
    return 0;
}