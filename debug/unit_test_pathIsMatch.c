#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../handler.c"

int main() {

    printf("/png == /png: %d\n", pathIsMatch("/png", "/png"));
    printf("/png/ == /png: %d\n", pathIsMatch("/png/", "/png"));
    printf("/png/dk == /png: %d\n", pathIsMatch("/png/dk", "/png"));

    return 0;
}