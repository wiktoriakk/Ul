#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while(running) {
	
		//losowe wejscie do ula	
		sem_t* entrance = (rand() % 2 ==0) ? &entrance1 : &entrance2;

		if (sem_wait(entrance) != 0) {
			perror("sem_wait");
			break;
		}
		
	        if (pthread_mutex_lock(&hive->lock) != 0) {
        	    perror("pthread_mutex_lock");
            		sem_post(entrance);
        	    break;
        	}
		
		//sprawdzenie liczby pszczol w ulu
		if (hive->bees_in_hive >= hive->max_bees_in_hive) {
			pthread_mutex_unlock(&hive->lock);
			sem_post(entrance);
			usleep(100000);
			continue;
		}

		int alive_bees = 0;
		int inside_hive = 0;

        	for (int i = 0; i < hive->total_bees; i++) {
			hive->bees[i].age++;

			if (hive->bees[i].type == 'W') {
				hive->bees[i].visits++;
				if(hive->bees[i].visits >hive->worker_lifespan) {
				printf("Pszczola %d umarla z powodu starosci.\n", i);
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

        if (pthread_mutex_unlock(&hive->lock) != 0) {
            perror("pthread_mutex_unlock");
            sem_post(entrance);
            break;
        }

        if (sem_post(entrance) != 0) {
            perror("sem_post");
            break;
        }

	usleep(700000);

	}

return NULL;

}
