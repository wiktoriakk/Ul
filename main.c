#include<stdio.h>
#include<stdlib.h>
#include"beehive.h"
#include<semaphore.h>

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

	pthread_mutex_init(&hive->lock, NULL);
	sem_init(&entrance1, 0, 1);
        sem_init(&entrance2, 0, 1);

	
	pthread_t queen, workers, beekeeper, monitor;
	pthread_create(&queen, NULL, queen_thread, hive);
	pthread_create(&workers, NULL, worker_thread, hive);
	pthread_create(&beekeeper, NULL, beekeeper_thread, hive);

	pthread_join(queen, NULL);
	pthread_cancel(workers);
	pthread_cancel(beekeeper);

	printf("\n--- Podsumowanie ---\n");
	printf("Calkowita liczba pszczol: %d\n", hive->total_bees);

	pthread_mutex_destroy(&hive->lock);
	sem_destroy(&entrance1);
	sem_destroy(&entrance2);
	free(hive->bees);
	free(hive);

	return 0;
}
