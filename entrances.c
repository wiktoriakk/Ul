#include "entrances.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "beehive.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdbool.h>

//inicjalizacja wejścia do ula
void init_entrance(Entrance* entrance) {
    entrance->sem_id_in = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (entrance->sem_id_in == -1) {
        perror("Błąd podczas tworzenia semafora dla wejścia");
        exit(EXIT_FAILURE);
    }

    if (semctl(entrance->sem_id_in, 0, SETVAL, 1) == -1) {
        perror("Błąd podczas ustawiania wartości semafora dla wejścia");
        exit(EXIT_FAILURE);
    }

	entrance->sem_id_out = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (entrance->sem_id_out == -1) {
        perror("Błąd podczas tworzenia semafora dla wychodzenia");
        exit(EXIT_FAILURE);
    }
    if (semctl(entrance->sem_id_out, 0, SETVAL, 1) == -1) {
        perror("Błąd podczas ustawiania wartości semafora dla wychodzenia");
        exit(EXIT_FAILURE);
    }

    entrance->entry_direction = true;
}

//inicjalizacja globalnego semafora do synchronizacji ula
void init_global_semaphore() {
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Błąd podczas tworzenia globalnego semafora");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("Błąd podczas ustawiania wartości globalnego semafora");
        exit(EXIT_FAILURE);
    }
}

//czyszczenie zasobów po wejściu do ula
void destroy_entrance(Entrance* entrance) {
    if (semctl(entrance->sem_id_in, 0, IPC_RMID) == -1) {
        perror("Błąd podczas usuwania semafora dla wejścia");
    }

	if (semctl(entrance->sem_id_out, 0, IPC_RMID) == -1) {
        perror("Błąd podczas usuwania semafora dla wychodzenia");
    }
}

//czyszczenie globalnego semafora
void destroy_global_semaphore() {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("Błąd podczas usuwania globalnego semafora");
    }
}

//funkcja umożliwiająca korzystanie z wejścia do ula
void use_entrance(Entrance* entrance, bool direction, int bee_id) {
    struct sembuf sb_lock = {0, -1, 0}; //zablokowanie wejścia
    struct sembuf sb_unlock = {0, 1, 0}; //odblokowanie wejścia

    int sem_id = direction ? entrance->sem_id_in : entrance->sem_id_out;

    while (semop(sem_id, &sb_lock, 1) == -1) {
        if (errno == EINTR) {
            printf("Pszczoła %d: semafor przerwany przez sygnał. Ponawianie...\n", bee_id);
            fflush(stdout);
            continue; 
        } else {
            perror("Błąd podczas blokowania semafora w use_entrance");
            exit(EXIT_FAILURE);
        }
    }

    printf("Pszczoła %d zablokowała semafor %s.\n", 
           bee_id, direction ? "wejścia" : "wyjścia");
    fflush(stdout);

    //jesli kierunek to wejście
    if (direction) {
        if (hive->bees_in_hive >= hive->max_bees_in_hive) {
            printf("Pszczoła %d nie może wejść: ul pełny (%d/%d).\n", 
                   bee_id, hive->bees_in_hive, hive->max_bees_in_hive);
        } else {
            hive->bees_in_hive++;
	    hive->bees_entered++;
            printf("Pszczoła %d wchodzi do ula. Liczba pszczół w ulu: %d/%d.\n", 
                   bee_id, hive->bees_in_hive, hive->max_bees_in_hive);
        }
    } 
    //jeśli kierunek to wyjście
    else {
        hive->bees_in_hive--;
	hive->bees_exited++;
        printf("Pszczoła %d wychodzi z ula. Liczba pszczół w ulu: %d/%d.\n", 
               bee_id, hive->bees_in_hive, hive->max_bees_in_hive);
    }
    fflush(stdout);

    if (semop(sem_id, &sb_unlock, 1) == -1) {
        perror("Błąd podczas odblokowywania semafora w use_entrance");
        exit(EXIT_FAILURE);
    }

    printf("Pszczoła %d zwolniła semafor %s.\n", 
           bee_id, direction ? "wejścia" : "wyjścia");
    fflush(stdout);
}
    
