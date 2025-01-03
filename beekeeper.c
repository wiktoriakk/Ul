#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* beekeeper_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;

	while (1) {
		pthread_mutex_lock(&hive->lock);

		if (hive->total_bees<hive->max_population) {
			int new_bees=3;
			for (int i=0;i<new_bees && hive->total_bees <hive->max_population; i++) {
			hive->bees[hive->total_bees].type = 'W';
			hive->bees[hive->total_bees].age = 0;
			hive->bees[hive->total_bees].visits = 0;
			hive->total_bees++;
			}
			printf("Pszczelarz dodaje 3 pszczoly. Liczba pszczol: %d\n", hive->total_bees);
		}
		pthread_mutex_unlock(&hive->lock);
		usleep(800000);
	}
	return NULL;
}
