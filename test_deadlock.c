#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

#define PROGRAM_PATH "./beehive"

bool is_running(pid_t pid) {
    return kill(pid, 0) == 0;
}

int main() {
    printf("Test: Zakleszczenie (deadlock)...\n");
    pid_t pid = fork();

    if (pid == 0) {
        execl(PROGRAM_PATH, PROGRAM_PATH, NULL);
        perror("Błąd podczas uruchamiania programu");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Błąd podczas tworzenia procesu");
        return EXIT_FAILURE;
    }

    sleep(2);

    printf("Wstrzymywanie procesu...\n");
    kill(pid, SIGSTOP); 

    sleep(3); 

    printf("Wznawianie procesu...\n");
    kill(pid, SIGCONT); 

    sleep(2);

    if (is_running(pid)) {
        printf("Test: ZALICZONY (Brak zakleszczenia, proces działa)\n");
    } else {
        printf("Test: NIE ZALICZONY (Proces zakończył działanie)\n");
    }

    kill(pid, SIGINT); // Zatrzymaj proces
    wait(NULL);

    return EXIT_SUCCESS;
}
