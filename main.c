#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"queen.h"
#include"worker.h"
#include"beekeeper.h"
#include"monitor.h"
#include<semaphore.h>

//struktura pszczoly
typedef struct {
	char type; //rodzaj pszczoly: krolowa czy robotnica
	int age; //wiek (zycie)
	int visits; //liczba wizyt w ulu
} Bee;

//struktura ula
typedef struct {
	Bee* bees; 
	int total_bees; //liczba pszczol w roju
	int max_population; //maksymalna liczba pszczol jaka moze pomiescic ul
	int max_bees_in_hive; //maksymalna liczba pszczol w ulu
	int queen_alive;
	pthread_mutex_t lock;
	sem_t entrance1, entrance2; 
} Beehive; 


int main() {
	int initial_bees = 10;
	int max_population = 50;
	int max_bees_in_hive = 25;

	Beehive* hive = malloc(sizeof(Beehive));
	hive->bees = malloc(max_population * sizeof(Bee));
	sem_init(&hive->entrance1, 0, 1);
	sem_init(&hive->entrance2, 0, 1);
	pthread_mutex_init(&hive->lock, NULL);
	hive->total_bees = initial_bees;
	hive->max_population = max_population;
	hive->max_bees_in_hive = max_bees_in_hive;
	hive->queen_alive = 1;
	pthread_mutex_init(&hive->lock, NULL);

	//printf("Poczatkowa liczba pszczol: %d\n", hive->total_bees);

	pthread_t queen_thread_id;
	if (pthread_create(&queen_thread_id, NULL, queen_thread, hive) != 0) {
		perror("Nie udalo sie stworzyc watku krolowej.");
		free(hive->bees);
		free(hive);
		return EXIT_FAILURE;
	}
	pthread_t worker_thread_id;
	if (pthread_create(&worker_thread_id, NULL, worker_thread, hive) != 0) {
		perror("Nie udalo sie stworzyc watku robotnicy.");
		free(hive->bees);
		free(hive);
		return EXIT_FAILURE;
	}

	pthread_t beekeeper_thread_id;
	if (pthread_create(&beekeeper_thread_id, NULL, beekeeper_thread, hive) != 0) {
    		perror("Nie udalo sie stworzyć watku pszczelarza.");
    		free(hive->bees);
    		free(hive);
    		return EXIT_FAILURE;
	}
	
	pthread_t monitor_thread_id;
	if (pthread_create(&monitor_thread_id, NULL, monitor_thread, hive) != 0) {
		perror("Nie udalo sie stworzyc watku monitorujacego.");
		return EXIT_FAILURE;
	}

	pthread_join(queen_thread_id, NULL);
	pthread_join(worker_thread_id, NULL);
	pthread_join(beekeeper_thread_id, NULL);
	pthread_join(monitor_thread_id, NULL);
	pthread_mutex_destroy(&hive->lock);
	free(hive->bees);
	free(hive);

	return 0;
}
