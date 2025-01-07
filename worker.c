#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while(1) {
		sem_wait(&entrance1);
		pthread_mutex_lock(&hive->lock);

		int alive_bees = 0;
		int inside_hive = 0;

        	for (int i = 0; i < hive->total_bees; i++) {
			hive->bees[i].age++;
			if (hive->bees[i].type == 'W') {
			hive->bees[i].visits++;

			if(hive->bees[i].visits >hive->worker_lifespan) {
				printf("Przegrzanie ula! Pszczola %d umarla z powodu przegrzania.\n", i);
				continue;
			}
			
			if (hive->bees[i].Ti == 0) {
				hive->bees[i].Ti = (rand() % 5 + 1)*1000;
			}
			if (hive->bees[i].Ti>0) {
				hive->bees[i].Ti -= 700;
				if (hive->bees[i].Ti <=0) {
					printf("Pszczola %d opuscila ul.\n", i);
				}
			}
			hive->bees[alive_bees++] = hive->bees[i];
			
			if (inside_hive < hive->max_bees_in_hive && hive->bees[i].Ti >0) {
				inside_hive++;			
			}
			} else if (hive->bees[i].type == 'Q') {
				hive->bees[alive_bees++] = hive->bees[i];
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
