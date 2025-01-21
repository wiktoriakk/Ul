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
    printf("Test: Stabilność w długim czasie...\n");
    pid_t pid = fork();

    if (pid == 0) {
        execl(PROGRAM_PATH, PROGRAM_PATH, NULL);
        perror("Błąd podczas uruchamiania programu");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Błąd podczas tworzenia procesu");
        return EXIT_FAILURE;
    }

    int duration = 60; //czas testu w sekundach
    printf("Uruchamianie symulacji na %d sekund...\n", duration);

    for (int i = 0; i < duration; i++) {
        sleep(1);
        if (!is_running(pid)) {
            printf("Test: NIE ZALICZONY (Proces zakończył działanie przed czasem)\n");
            return EXIT_FAILURE;
        }
    }

    printf("Test: ZALICZONY (Proces działa stabilnie przez %d sekund)\n", duration);

    kill(pid, SIGINT); 
    wait(NULL);

    return EXIT_SUCCESS;
}
