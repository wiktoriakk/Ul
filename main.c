#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include "beehive.h"
#include"entrances.h"
#include<errno.h>

//zmienne globalne
int running = 1;
int shm_id;
int sem_id;
Beehive *hive;

void signal_handler(int signo) {
        if (signo == SIGUSR1) {
                printf("Otrzymano sygnał SIGUSR1: Dodanie ramek.\n");
                hive->frame_signal = 1;
        } else if (signo == SIGUSR2) {
                printf("Otrzymano sygnł SIGUSR2: Usunięcie ramek.\n");
                hive->frame_signal = 2;
        } else if (signo == SIGINT) {
		printf("Otrzymano sygnał SIGINT: Zatrzymanie symulacji.\n");
                running = 0;
        }
}


void validate_hive(Beehive *hive) {
        if (hive->total_bees <= 0 || hive->max_population <= 0 || hive->max_bees_in_hive <= 0) {
                fprintf(stderr, "Błąd: Wszystkie wartości muszą być dodatnie.\n");
                exit(EXIT_FAILURE);
        }
        if (hive->max_bees_in_hive>hive->max_population) {
                fprintf(stderr, "Błąd: Maksymalna liczba pszczół w ulu nie może przekraczać maksymalnej populacji.\n");
                exit(EXIT_FAILURE);
        }
        if (hive->max_bees_in_hive >= (hive->total_bees / 2)) {
                fprintf(stderr, "Błąd: Maksymalna liczba pszczół w ulu musi być mniejsza niż połowa początkowej liczby pszczół.\n");
                exit(EXIT_FAILURE);
        }
}


void start_process(void (*process_func)()) {
	pid_t pid = fork();
	if (pid == 0) {
		process_func();	
		exit(0);
	} else if (pid<0) {
		perror("Błąd podczas fork");
		exit(EXIT_FAILURE);
	}
}

int main() {
	shm_id = shmget(IPC_PRIVATE, sizeof(Beehive), IPC_CREAT | 0666);
	if (shm_id < 0) {
		perror("Błąd podczas tworzenia pamięci współdzielonej");
		exit(EXIT_FAILURE);
	}
	hive=(Beehive *)shmat(shm_id, NULL, 0);
	if (hive==(void *)-1) {
		perror("Błąd podczas dołączania pamięci współdzielonej");
		exit(EXIT_FAILURE);
	}

        hive->total_bees = 50;
        hive->max_population = 100;
        hive->max_bees_in_hive = 24;
        hive->bees_in_hive = 15;
        hive->queen_alive = 1;
        hive->frame_signal = 0;

	validate_hive(hive);

        for (int i=0; i< hive->total_bees; i++) {
		hive->bees[i].id = i;
                hive->bees[i].type = (i==0) ? 'Q' : 'W';
                hive->bees[i].age = 0;
                hive->bees[i].visits = 0;
                hive->bees[i].Ti = (rand() % 5 + 1)* 1000;
		hive->bees[i].outside = (i >= hive->bees_in_hive);
	}
       
	init_entrance(&hive->entrance1);
    	init_entrance(&hive->entrance2);
        
	sem_id=semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (sem_id < 0) {
		perror("Błąd podczas tworzenia semafora");
		exit(EXIT_FAILURE);
	}
	if (semctl(sem_id, 0, SETVAL, 1) == -1) {
		perror("Błąd podczas ustawiania wartości semafora");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
		perror("Błąd podczas ustawiania obsługi SIGUSR1");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGUSR2, signal_handler) == SIG_ERR) {
		perror("Błąd podczas ustawiania obsługi SIGUSR2");
		exit(EXIT_FAILURE);
	}
	
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		perror("Błąd podczas ustawiania obsługi SIGINT");
		exit(EXIT_FAILURE);
	}

	start_process(queen_process);
	start_process(worker_process);
	start_process(beekeeper_process);
	start_process(monitor_process);

	while(running) {
		sleep(1);
	}

        if (kill(0, SIGKILL) == -1) {
		perror("Błąd podczas wysyłania sygnału SIGKILL do procesów potomnych");
	}

	if (shmdt(hive) == -1) {
		perror("Błąd podczas odłączania pamięci współdzielonej");
	}

	if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
		perror("Błąd podczas usuwania pamięci współdzielonej");
	}
	
	if (semctl(sem_id, 0, IPC_RMID) == -1) {
		perror("Błąd podczas usuwania semafora");
	}

	destroy_entrance(&hive->entrance1);
	destroy_entrance(&hive->entrance2);

	printf("Symulacja została zakończona.\n");
	
	return 0;
}
