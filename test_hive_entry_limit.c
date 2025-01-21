#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define PROGRAM_PATH "./beehive"

int main() {
    printf("Test: Blokada wejścia do ula po osiągnięciu limitu...\n");
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

    printf("Wysyłanie sygnałów w celu zapełnienia ula...\n");
    for (int i = 0; i < 20; i++) { 
        kill(pid, SIGUSR1);
        usleep(500000); 
    }

    printf("Sprawdź logi programu. Test ZALICZONY, jeśli ul odmawia wstępu po osiągnięciu limitu.\n");

    kill(pid, SIGINT);
    wait(NULL);

    return EXIT_SUCCESS;
}
