#include<stdio.h>
#include<sys/sem.h>
#include<stdlib.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>

extern int running;

void monitor_process() {
    printf("Rozpoczęcie monitorowania ula...\n");
    FILE *summary_file = NULL;

    while (running) {
	printf("Monitor: Pętla monitorowania trwa...\n");

        struct sembuf sb = {0, -1, 0};
        while (semop(sem_id, &sb, 1) == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("Błąd podczas pobierania semafora w monitor_process");
                exit(EXIT_FAILURE);
            }
        }

        if (pthread_mutex_lock(&hive->mutex) != 0) {
            perror("Błąd podczas blokowania mutexu w monitor_process");
            sb.sem_op = 1;
            if (semop(sem_id, &sb, 1) == -1) {
                perror("Błąd podczas zwalniania semafora w monitor_process");
            }
            exit(EXIT_FAILURE);
        }


        //sprawdzenie czy ul zakoćzył pracę
        if (hive->bees_in_hive == 0 || hive->queen_alive == 0) {
            printf("Monitor: Ul zakończył pracę. Tworzenie podsumowania...\n");
            running = 0;

            //zapis podsumowania do pliku
	    summary_file = fopen("hive_summary.log", "w");
            if (summary_file == NULL) {
                perror("Nie można otworzyć pliku do zapisu podsumowania");
            } else {
                fprintf(summary_file, "--- Podsumowanie symulacji ---\n");
                fprintf(summary_file, "Królowa: %s\n", hive->queen_alive ? "żyje" : "umarła");
                fprintf(summary_file, "Całkowita liczba pszczół: %d\n", hive->total_bees);
                fprintf(summary_file, "Pszczoły w ulu na końcu symulacji: %d\n", hive->bees_in_hive);
                fprintf(summary_file, "Wejście 1: %d pszczół (ostatni kierunek: %s)\n",
                        hive->entrance1.bees_inside,
                        hive->entrance1.entry_direction ? "wejście" : "wyjście");
                fprintf(summary_file, "Wejście 2: %d pszczół (ostatni kierunek: %s)\n",
                        hive->entrance2.bees_inside,
                        hive->entrance2.entry_direction ? "wejście" : "wyjście");
                fprintf(summary_file, "Liczba złożonych jaj: %d\n", hive->eggs_laid);
                fprintf(summary_file, "Liczba pszczół, które weszły do ula: %d\n", hive->bees_entered);
                fprintf(summary_file, "Liczba pszczół, które wyszły z ula: %d\n", hive->bees_exited);

		printf("Zapis podsumowania zakończony.");

                if (fclose(summary_file) != 0) {
                    perror("Nie można zamknąć pliku z podsumowaniem");
                }
            }

            if (pthread_mutex_unlock(&hive->mutex) != 0) {
                perror("Błąd podczas odblokowywania mutexu w monitor_process");
            }
            sb.sem_op = 1;
            if (semop(sem_id, &sb, 1) == -1) {
                perror("Błąd podczas zwalniania semafora w monitor_process");
            }
            break;
        }

        //wyświetlenie aktualnego stanu ula
        printf("\033[35m\n--- Stan ula ---\033[0m\n");
        printf("Królowa: %s\n", hive->queen_alive ? "żyje" : "umarła");
        printf("Całkowita liczba pszczół: %d\n", hive->total_bees);
        printf("Pszczoły w ulu: %d\n", hive->bees_in_hive);
        printf("Wejście 1: %d pszczół (kierunek: %s)\n",
               hive->entrance1.bees_inside,
               hive->entrance1.entry_direction ? "wejście" : "wyjście");
        printf("Wejście 2: %d pszczół (kierunek: %s)\n",
               hive->entrance2.bees_inside,
               hive->entrance2.entry_direction ? "wejście" : "wyjście");

        if (pthread_mutex_unlock(&hive->mutex) != 0) {
            perror("Błąd podczas odblokowywania mutexu w monitor_process");
            sb.sem_op = 1;
            if (semop(sem_id, &sb, 1) == -1) {
                perror("Błąd podczas zwalniania semafora w monitor_process");
            }
            exit(EXIT_FAILURE);
        }

        sb.sem_op = 1;
        if (semop(sem_id, &sb, 1) == -1) {
            perror("Błąd podczas zwalniania semafora w monitor_process");
            exit(EXIT_FAILURE);
        }

        sleep(5);
    }

    printf("Proces monitorowania zakończył działanie.\n");
}
