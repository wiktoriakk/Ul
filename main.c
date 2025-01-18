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

/*
void setup_signal_handlers() {
	signal(SIGUSR1, handle_signal);
	signal(SIGUSR2, handle_signal);
	signal(SIGINT, handle_signal);
}


void validate_input(int initial_bees, int max_population, int max_bees_in_hive) {
        if (initial_bees <= 0 || max_population <= 0 || max_bees_in_hive <= 0) {
                fprintf(stderr, "Blad: Wszystkie wartosci musza byc dodatnie.\n");
                exit(EXIT_FAILURE);
        }
        if (max_bees_in_hive>max_population) {
                fprintf(stderr, "Blad: Maksymalna liczba pszczol w ulu nie moze przekraczac maksymalnej populacji.\n");
                exit(EXIT_FAILURE);
        }
        if (max_bees_in_hive >= (initial_bees / 2)) {
                fprintf(stderr, "Blad: Maksymalna liczba pszczol w ulu musi byc mniejsza niz polowa poczatkowej liczby pszczol.\n");
                exit(EXIT_FAILURE);
        }
}
*/

void start_process(void (*process_func)()) {
	pid_t pid = fork();
	if (pid == 0) {
		process_func();	
		exit(0);
	} else if (pid<0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
}

int main() {
	shm_id = shmget(IPC_PRIVATE, sizeof(Beehive), IPC_CREAT | 0666);
	if (shm_id < 0) {
		perror("shmget");
		exit(EXIT_FAILURE);
	}
	hive=(Beehive *)shmat(shm_id, NULL, 0);
	if (hive==(void *)-1) {
		perror("shmat");
		exit(EXIT_FAILURE);
	}

        hive->total_bees = 50;
        hive->max_population = 100;
        hive->max_bees_in_hive = 24;
        hive->bees_in_hive = 15;
        hive->queen_alive = 1;
        hive->frame_signal = 0;

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
		perror("semget");
		exit(EXIT_FAILURE);
	}
	semctl(sem_id, 0, SETVAL, 1);

	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGINT, signal_handler);

	start_process(queen_process);
	start_process(worker_process);
	start_process(beekeeper_process);
	start_process(monitor_process);

	while(running) {
		sleep(1);
	}

        kill(0, SIGKILL);

	shmdt(hive);
	shmctl(shm_id, IPC_RMID, NULL);

	semctl(sem_id, 0, IPC_RMID);

	destroy_entrance(&hive->entrance1);
	destroy_entrance(&hive->entrance2);

	printf("Symulacja została zakończona.\n");
	
	return 0;
}
