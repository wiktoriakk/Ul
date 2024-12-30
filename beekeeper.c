#include<stdio.h>
#include<unistd.h>
#include"beekeeper.h"

void* beekeeper_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;

	while (hive->queen_alive) {
		pthread_mutex_lock(&hive->lock);

		if (hive->total_bees<hive->max_bees_in_hive) {
			hive->bees[hive->total_bees].type = 'W';
			hive->bees[hive->total_bees].age = 0;
			hive->bees[hive->total_bees].visits = 0;
			hive->total_bees++;
			printf("Pszczelarz dodaje nowa pszczole. Liczba pszczol: %d\n", hive->total_bees);
		}
		pthread_mutex_unlock(&hive->lock);
		sleep(2);
	}
	return NULL;
}
