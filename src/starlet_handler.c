#include <sys/wait.h>

#define MAX_XML_SIZE 8*4096
#define DTD_FILE "addressbook.dtd"

void xmlstarlet_server() {

    if (-1 == setgid(666)) {
        printf("Could not drop privileges\n");
        exit(1);
    }

    if (-1 == setuid(666)) {
        printf("Could not change user\n");
        exit(1);
    }

    while(1) {

        int req = open("api/xmlvalreq", O_RDONLY);
        int res = open("api/xmlvalres", O_WRONLY);

        int child = fork();
        if (0 == child) {
            execlp("xmlstarlet", "xmlstarlet", "val", "-d", DTD_FILE, "-", NULL);
            exit(0);
        } else {
            close(req);
            close(res);

            waitpid(child, 0, 0);
        }
    }
    exit(0);
}