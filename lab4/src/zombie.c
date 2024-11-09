#include <stdio.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char** argv) {
    int pid = fork();
    if (pid == 0) {
        puts("zombie");
        return 0;
    }
    
    for (int i = 1; i < 6; i++) {
        sleep(1);
        printf("%d\n", i);
    }

    wait(0);
}