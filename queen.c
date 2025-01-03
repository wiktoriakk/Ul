#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* queen_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while (1) {
        	pthread_mutex_lock(&hive->lock);
		
		if(!hive->queen_alive) {
			printf("Krolowa umarla!\n");
			pthread_mutex_unlock(&hive->lock);
			break;
		}

        	if (hive->total_bees < hive->max_population) {
		int eggs;
		for(int i=0;i<eggs && hive->total_bees<hive->max_population;i++)
            		hive->bees[hive->total_bees].type = 'W';
            		hive->bees[hive->total_bees].age = 0;
            		hive->bees[hive->total_bees].visits = 0;
            		hive->total_bees++;
            	printf("Krolowa sklada 5 jaj. Liczba pszczol: %d\n", hive->total_bees);
		}

		hive->bees[0].age++;
		if (hive->bees[0].age>hive->queen_lifespan) {
			hive->queen_alive=0;

		}

        	pthread_mutex_unlock(&hive->lock);
        	usleep(500000);
    	}

    return NULL;
}
