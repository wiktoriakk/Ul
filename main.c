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
#include<errno.h>

Beehive* hive;

sem_t entrance1, entrance2;
int running = 1;

void handle_error(const char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void signal_handler(int signo) {
	if (signo == SIGUSR1) {
		printf("Otrzymano sygnal SIGUSR1: Dodanie ramek.\n");
		hive->frame_signal = 1;
	} else if (signo == SIGUSR2) {
		printf("Otrzymano sygnal SIGUSR2: Usuniecie ramek.\n");
		hive->frame_signal = 2;
	} else if (signo == SIGINT) {
		running = 0;
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
	if (!hive) {
		handle_error("malloc");
	}

	hive->bees = malloc(max_population * sizeof(Bee));
	if (!hive) {
		handle_error("malloc");
	}

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

	if (pthread_mutex_init(&hive->lock, NULL) != 0) {
		handle_error("pthread_mutex_init");
	}
	if (sem_init(&entrance1, 0, 1) != 0 || sem_init(&entrance2, 0, 1) != 0) {
		handle_error("sem_init");
	}
        
	if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
		handle_error("signal");
	}
	if (signal(SIGUSR2, signal_handler) == SIG_ERR) {
		handle_error("signal");
	}

	pthread_t queen, workers, beekeeper, monitor;

	if (pthread_create(&queen, NULL, queen_thread, hive) != 0) {
		handle_error("pthread_create");
	}
	if (pthread_create(&workers, NULL, worker_thread, hive) != 0) {
		handle_error("pthread_create");
	}
	if (pthread_create(&beekeeper, NULL, beekeeper_thread, hive) != 0) {
		handle_error("pthread_create");
	}
	if (pthread_create(&monitor, NULL, monitor_thread, hive) != 0) {
		handle_error("pthread_create");
	}

	if (pthread_join(queen, NULL) != 0) {
		handle_error("pthread_join");
	}
	
	if (pthread_cancel(workers) != 0) {
		handle_error("pthread_cancel");
	}
	if (pthread_cancel(beekeeper) != 0) {
		handle_error("pthread_cancel");
	}
	if (pthread_cancel(monitor) != 0) {
		handle_error("pthread_cancel");
	}

	printf("\n--- Podsumowanie ---\n");
	printf("Calkowita liczba pszczol: %d\n", hive->total_bees);
	printf("Krolowa: %s\n", hive->queen_alive ? "zyje" : "umarla");
	printf("Pszczoly w ulu: %d\n", hive->bees_in_hive);

	if (pthread_mutex_destroy(&hive->lock) != 0) {
		handle_error("pthread_mutex_destroy");
	}
	if (sem_destroy(&entrance1) != 0 || sem_destroy(&entrance2) != 0) {
        handle_error("sem_destroy");
    	}

	free(hive->bees);
	free(hive);

	return 0;
}
