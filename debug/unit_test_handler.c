#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../handler.c"

int main() {

    close(0);
    open("debug/get-request.txt", O_RDONLY);

    handle_request();
}