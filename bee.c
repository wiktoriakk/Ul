#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include "beehive.h"
#include "entrances.h"
#include<errno.h>


extern int running;

void bee_process(void *arg) {
    Bee *bee = (Bee *)arg;

    printf("Proces pszczoły %d rozpoczął działanie (PID: %d).\n", bee->id, getpid());
    fflush(stdout);
 
    while (running && !bee->dead) { //główna pętla
        struct sembuf sb = {0, -1, 0};

        while (semop(sem_id, &sb, 1) == -1) { //pobieranie semafora
            if (errno == EINTR) {
                printf("Pszczoła %d: semafor przerwany przez sygnał. Ponawianie...\n", bee->id);
                fflush(stdout);
                continue;
            } else {
                perror("Błąd podczas blokowania semafora w bee_process");
                exit(EXIT_FAILURE);
            }
        }

        if (pthread_mutex_lock(&hive->mutex) != 0) { 
            perror("Błąd podczas blokowania mutexu w bee_process");
            sb.sem_op = 1;
            if (semop(sem_id, &sb, 1) == -1) {
                perror("Błąd podczas zwalniania semafora w bee_process");
            }
            exit(EXIT_FAILURE);
        }

        //wyświetlenie i aktualizacja stanu pszczoły
        printf("Pszczoła %d: outside=%d, Ti=%d, visits=%d, age=%d, bees_in_hive=%d/%d\n",
               bee->id, bee->outside, bee->Ti, bee->visits, bee->age, hive->bees_in_hive, hive->max_bees_in_hive);
        fflush(stdout);
	printf("\n");
	
        if (bee->outside) {
            //jeśli pszczoła próbuje wejść do ula
            if (hive->bees_in_hive < hive->max_bees_in_hive) { //jeśli liczba pszczół w ulu jest mniejsza niż maksymalna liczba pszczół w ulu
                printf("Pszczoła %d znajduje się na zewnątrz i próbuje wejść do ula.\n", bee->id);
		
                use_entrance(&hive->entrance1, true, bee->id); //funkcja obsługująca wejscie do ula
                hive->bees_in_hive++; //zwiększenie liczby pszczół w ulu
                bee->outside = false; //pszczoła jest teraz w ulu
                bee->Ti = (rand() % 5 + 5) * 1000; //generowany losowy nowy czas pobytu w ulu
            } else {
                printf("Pszczoła %d nie może wejść: ul pełny.\n", bee->id);
                fflush(stdout);
            }
        } else {
            //pszczoła jest w ulu i zmniejsza czas pobytu
            bee->Ti -= 500;
            if (bee->Ti <= 0) {
                printf("Pszczoła %d: kończy pobyt w ulu i wychodzi na zewnątrz.\n", bee->id);
                use_entrance(&hive->entrance2, false, bee->id); //funkcja obsługująca wyjście z ula
                hive->bees_in_hive--; //zmniejszenie liczby pszczół w ulu
                bee->outside = true; //pszczoła jest poza ulem
            }
        }

        //aktualizacja wieku i liczby wizyt pszczoły
        bee->visits++;
        bee->age++;

        //sprawdzenie czy pszczoła umiera po określonej liczbie wizyt = 3
        if (bee->visits > 3) {
            printf(BLUE "Pszczoła %d umiera ze starości." RESET "\n", bee->id);
            bee->dead = true; //pszczoła jest martwa
            hive->bees_in_hive--; //zmniejszenie liczby pszczół w ulu
        }

        if (pthread_mutex_unlock(&hive->mutex) != 0) {
            perror("Błąd podczas odblokowywania mutexa w bee_process");
        }

        sb.sem_op = 1;
        if (semop(sem_id, &sb, 1) == -1) {
            perror("Błąd podczas zwalniania semafora w bee_process");
            exit(EXIT_FAILURE);
        }

        usleep(800000); 
    }

    printf("Proces pszczoły %d zakończył działanie (PID: %d).\n", bee->id, getpid());
    fflush(stdout);
    exit(0);
}


pid_t start_bee_process(Bee *bee) { //tworzenie procesu pszczoły
    pid_t pid = fork();
    if (pid == 0) {
        bee_process(bee); //proces potomny: uruchomienie funkcji pszczoły
        exit(0);
    } else if (pid < 0) {
        perror("Błąd podczas tworzenia procesu pszczoły");
        return -1;
    }
    bee->process_id = pid; 
    return pid; //zwraca pid procesu pszczoły
} 
