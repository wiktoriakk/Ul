#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while(1) {
		sem_wait(&entrance1);
		pthread_mutex_lock(&hive->lock);

		int alive_bees = 1;
		int inside_hive = 0;

        	for (int i = 1; i < hive->total_bees; i++) {
			hive->bees[i].age++;
			hive->bees[i].visits++;

			if(hive->bees[i].visits >hive->worker_lifespan) {
				printf("Przegrzanie ula! Pszczola %d umarla z powodu przegrzania.\n", i);
				continue;
			}
			
			if (hive->bees[i].age<=hive->worker_lifespan) {

            		hive->bees[alive_bees++] = hive->bees[i];
			if (inside_hive < hive->max_bees_in_hive) {
				inside_hive++;			
			}
			}
		}

		hive->total_bees=alive_bees;
		hive->bees_in_hive=inside_hive;

	pthread_mutex_unlock(&hive->lock);
	sem_post(&entrance1);
	usleep(700000);

	}

return NULL;

}
