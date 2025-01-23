#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
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

        // Sprawdzenie pliku logów
        struct stat st;
        if (stat("simulation_log.txt", &st) == 0) {
            printf("Plik logów istnieje i jego rozmiar to: %ld bajtów\n", st.st_size);
        } else {
            perror("Nie znaleziono pliku logów");
        }
    } else {
        perror("Błąd tworzenia procesu");
        exit(EXIT_FAILURE);
    }

    return 0;
}
