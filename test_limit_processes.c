#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define PROGRAM_PATH "./beehive"

int main() {
    printf("Test: Przekroczenie maksymalnej liczby procesów...\n");
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

    printf("Sprawdzanie liczby procesów...\n");
    char command[128];
    snprintf(command, sizeof(command), "pgrep -P %d | wc -l", pid);
    FILE *fp = popen(command, "r");

    if (!fp) {
        perror("Błąd podczas sprawdzania liczby procesów");
        kill(pid, SIGINT);
        wait(NULL);
        return EXIT_FAILURE;
    }

    int process_count;
    fscanf(fp, "%d", &process_count);
    pclose(fp);

    if (process_count <= 4) {
        printf("Test: ZALICZONY (Liczba procesów: %d)\n", process_count);
    } else {
        printf("Test: NIE ZALICZONY (Przekroczono maksymalną liczbę procesów: %d)\n", process_count);
    }

    kill(pid, SIGINT); 
    wait(NULL);

    return EXIT_SUCCESS;
}
