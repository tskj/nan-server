#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {

	if (0 != fork()) exit(0);

	setsid();
	signal(SIGHUP, SIG_IGN);

	if (0 != fork()) exit(0);


	if (-1 == chdir("/www")) {
		printf("Could not change working directory\n");
		exit(1);
	}

	execl("/usr/bin/nodejs", "nodejs", "bin/server.js", NULL);
	printf("An error occured starting the nodejs server\n");

	return 1;
}
