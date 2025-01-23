#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

int main() {
    struct rlimit limit;
    getrlimit(RLIMIT_NPROC, &limit);
    printf("Maksymalna liczba procesów: %ld\n", limit.rlim_cur);

    pid_t pid = fork();

    if (pid == 0) {
        // Proces potomny - uruchomienie programu
        execl("./beehive", "./beehive", NULL);
        perror("Błąd uruchamiania programu");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Proces macierzysty - oczekiwanie na zakończenie
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
