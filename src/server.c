#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#include "handler.c"

#define LOCAL_PORT 80
#define QUEUE 10

void server() {

    struct sockaddr_in local_address;
    int sd, incomming_sd;

    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    local_address.sin_family        = AF_INET;
    local_address.sin_port          = htons((u_short) LOCAL_PORT);
    local_address.sin_addr.s_addr   = htons(          INADDR_ANY);
    
    if (-1 == bind(sd, (struct sockaddr *) &local_address, sizeof(local_address))) {
        printf("Could not bind to part %d\n", LOCAL_PORT);
    }

    if (-1 == setgid(666)) {
        printf("Could not drop privileges\n");
        exit(1);
    }

    if (-1 == setuid(666)) {
        printf("Could not change user\n");
        exit(1);
    }

    listen(sd, QUEUE);
    while (1) {

        incomming_sd = accept(sd, NULL, NULL);

        if (0 == fork()) {

            dup2(incomming_sd, 0);
            dup2(incomming_sd, 1);
            
            handle_request();

            exit(0);

        } else {
            close(incomming_sd);
        }
    }
}

int main() {

    if (0 != fork()) exit(0);

    setsid();
    signal(SIGHUP, SIG_IGN);

    if (0 != fork()) exit(0);

    if (-1 == chdir("/www")) {
        printf("Could not change working directory\n");
        exit(1);
    }

    if (-1 == chroot("/www")) {
        printf("Could not change root\n");
        exit(1);
    }

    int fd;
    for (fd = 0; fd < _NFILE; fd++) {
        close(fd);
    }

    signal(SIGCHLD, SIG_IGN);

    server();

    return 1;
}