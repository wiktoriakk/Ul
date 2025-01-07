#include<stdio.h>
#include<stdlib.h>
#include"beehive.h"
#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/msg.h>

sem_t entrance1, entrance2;

void signal_handler(int signo) {
	if (signo == SIGUSR1) {
		printf("Otrzymano sygnal SIGUSR1: Dodanie ramek.\n");
		hive->frame_signal = 1;
	} else if (signo == SIGUSR2) {
		printf("Otrzymano sygnal SIGUSR2: Usuniecie ramek.\n");
		hive->frame_signal = 2;
	}
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
}

int main() {
	int initial_bees = 10;
	int max_population = 50;
	int max_bees_in_hive = 25;
	int queen_lifespan = 20;
	int worker_lifespan = 10;

	validate_input(initial_bees, max_population, max_bees_in_hive);

	Beehive* hive = malloc(sizeof(Beehive));
	hive->bees = malloc(max_population * sizeof(Bee));
	hive->total_bees = initial_bees;
	hive->max_population = max_population;
	hive->max_bees_in_hive = max_bees_in_hive;
	hive->queen_lifespan = queen_lifespan;
	hive->worker_lifespan = worker_lifespan;
	hive->bees_in_hive = 0;
	hive->queen_alive = 1;
	hive->frame_signal = 0;
	
	srand(time(NULL));
	for (int i=0; i<initial_bees; i++) {
		hive->bees[i].type = 'W';
		hive->bees[i].age =0;
		hive->bees[i].visits = 0;
		hive->bees[i].Ti = (rand() % 5 + 1)* 1000;
	}
	hive->bees[0].type = 'Q';

	pthread_mutex_init(&hive->lock, NULL);
	sem_init(&entrance1, 0, 1);
        sem_init(&entrance2, 0, 1);
	
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	pthread_t queen, workers, beekeeper, monitor;
	pthread_create(&queen, NULL, queen_thread, hive);
	pthread_create(&workers, NULL, worker_thread, hive);
	pthread_create(&beekeeper, NULL, beekeeper_thread, hive);
	pthread_create(&monitor, NULL, monitor_thread, hive);

	pthread_join(queen, NULL);
	
	pthread_cancel(workers);
	pthread_cancel(beekeeper);
	pthread_cancel(monitor);

	printf("\n--- Podsumowanie ---\n");
	printf("Calkowita liczba pszczol: %d\n", hive->total_bees);
	printf("Krolowa: %s\n", hive->queen_alive ? "zyje" : "umarla");
	printf("Pszczoly w ulu: %d\n", hive->bees_in_hive);

	pthread_mutex_destroy(&hive->lock);
	sem_destroy(&entrance1);
	sem_destroy(&entrance2);
	free(hive->bees);
	free(hive);

	return 0;
}
