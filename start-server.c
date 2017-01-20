#include <signal.h>

int main() {

	if (0 != fork()) exit(0);

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	return 0;
}
