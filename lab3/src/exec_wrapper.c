#include <unistd.h>
// #include <sys/wait.h>

int main(int argc, char *argv[]) {
	if (fork() == 0)
        execvp("./sequential_min_max", argv);

    // wait(NULL);

	return 0;
}