#include<stdio.h>
#include<stdlib.h>
#include"beehive.h"
#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>

sem_t entrance1, entrance2;

int main() {
	int initial_bees = 10;
	int max_population = 50;
	int max_bees_in_hive = 25;
	int queen_lifespan = 20;
	int worker_lifespan = 10;

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
