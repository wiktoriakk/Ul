#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include"main.h"

void* queen_thread(void* arg) {

	Beehive* hive = (Beehive*)arg;

	while (hive->queen_alive) {
        	pthread_mutex_lock(&hive->lock);

        	if (hive->total_bees < hive->max_population) {
            	hive->bees[hive->total_bees].type = 'W';
            	hive->bees[hive->total_bees].age = 0;
            	hive->bees[hive->total_bees].visits = 0;
            	hive->total_bees++;
            	printf("Krolowa sklada jajo. Liczba pszczol: %d\n", hive->total_bees);
		}

        	pthread_mutex_unlock(&hive->lock);
        	sleep(1);
    	}

    printf("Krolowa umarla.\n");
    return NULL;
}
