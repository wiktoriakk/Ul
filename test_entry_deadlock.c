#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        execl("./beehive", "./beehive", NULL);
        perror("Błąd uruchamiania programu");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        sleep(5);

        printf("Wysyłanie sygnału SIGINT do programu\n");
        kill(pid, SIGINT);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Program zakończył się poprawnie z kodem: %d\n", WEXITSTATUS(status));
        } else {
            printf("Program zakończył się nieprawidłowo\n");
        }
    } else {
        perror("Błąd tworzenia procesu");
        exit(EXIT_FAILURE);
    }

    return 0;
}
