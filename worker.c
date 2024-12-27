#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include"shared.h"

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;
	while(1) {
		 pthread_mutex_lock(&hive->lock);

        	int alive_bees = 1; 
		for (int i = 1; i < hive->total_bees; i++) {
            		hive->bees[i].age++;
            		hive->bees[i].visits++;

            		if (hive->bees[i].age <= hive->worker_lifespan) {
                		hive->bees[alive_bees++] = hive->bees[i];
            		} else {
				printf("Pszczola %d umarla z powodu starosci.\n", i);
			}
		}
		hive->total_bees=alive_bees;
		
		pthread_mutex_unlock(&hive->lock);
		usleep(700000);
	}

return NULL;

}
