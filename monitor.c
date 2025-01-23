#include<stdio.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include "beehive.h"
#include <errno.h>

extern int running;

void monitor_process() {
    printf("Rozpoczęcie monitorowania ula...\n");

    while (running) { //główna pętla monitorowania
        struct sembuf sb = {0, -1, 0};
        if (semop(sem_id, &sb, 1) == -1) {
            perror("Błąd podczas pobierania semafora w monitor_process");
            exit(EXIT_FAILURE);
        }

        if (pthread_mutex_lock(&hive->mutex) != 0) {
            perror("Błąd podczas blokowania mutexu w monitor_process");
            sb.sem_op = 1;
            if (semop(sem_id, &sb, 1) == -1) {
                perror("Błąd podczas zwalniania semafora w monitor_process");
            }
            exit(EXIT_FAILURE);
        }

        int alive_bees = 0; //zliczanie aktywnych pszczół w roju
        for (int i = 0; i < hive->total_bees; i++) {
            if (!hive->bees[i].dead) {
                alive_bees++;
            }
        }
        printf("Monitor: Liczba aktywnych pszczół: %d\n", alive_bees);
//sprawdzenie warunków zakończenia symulacji
        if (hive->total_bees >= hive->max_population || (hive->bees_in_hive == 0 && alive_bees == 0)) {
            printf("Monitor: Zmieniam running na 0. Zatrzymywanie symulacji.\n");
            running = 0;
        }

        //wyświetlanie stanu ula, jeśli symulacja wciąż trwa
        if (running) {
	printf(CYAN "\n--- Stan ula ---" RESET "\n");
        printf("Królowa: %s\n", hive->queen_alive ? "żyje" : "umarła");
        printf("Całkowita liczba pszczół: %d\n", hive->total_bees);
        printf("Pszczoły w ulu: %d\n", hive->bees_in_hive);
        printf("Aktywne pszczóły: %d\n", alive_bees);
	
	sleep(1);
	}
	
        if (pthread_mutex_unlock(&hive->mutex) != 0) {
            perror("Błąd podczas odblokowywania mutexu w monitor_process");
        }

        sb.sem_op = 1;
        if (semop(sem_id, &sb, 1) == -1) {
            perror("Błąd podczas zwalniania semafora w monitor_process");
            exit(EXIT_FAILURE);
        }

        sleep(5);
    }

    printf("Proces monitorowania zakończył działanie (PID: %d).\n", getpid());
    fflush(stdout);
	exit(0);
}
