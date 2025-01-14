#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>
#include<stdbool.h>
#include"entrances.h"
#include<pthread.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<semaphore.h>

extern Entrance entrance1;
extern Entrance entrance2;
extern int running;

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while(running) {
		sem_wait(&hive->worker_semaphore);

		pthread_mutex_lock(&hive->lock);
		//aktualizacja danych pszczol	
		        	
		int alive_bees = 0;

		for (int i=0;i<hive->total_bees;i++) {
			int idx = rand() % hive->total_bees;

			hive->bees[idx].age++;
			
			//spr czy pszczola osiagnela swoj maksymalny wiek
			if (hive->bees[idx].type == 'W') {
				hive->bees[idx].visits++;
				if(hive->bees[idx].visits > hive->worker_lifespan) {
				printf("Pszczola %d umarla z powodu starosci.\n", hive->bees[idx].id);
				continue;
								
			}		
			if (hive->bees[idx].Ti == 0) {
				hive->bees[idx].Ti = (rand() % 5 + 5) * 1000;
			}
			//zmniejszenie czasu wewnatrz ila
			if (hive->bees[idx].Ti>0) {
				hive->bees[idx].Ti -= 500;
			}
			
			// wejscie lub wyjscie z ula
			bool entering = hive->bees[idx].outside;
            		if (entering) {
                		if (hive->bees[idx].outside) {
                    		use_entrance(&entrance1, true, hive->bees[idx].id);
                    		hive->bees_in_hive++;
                    		hive->bees[idx].outside = false;
                		}
            		} else {
                		if (!hive->bees[idx].outside) {
                    		use_entrance(&entrance2, false, hive->bees[idx].id);
                    		//printf("Pszczoła %d opuszcza ul przez wejście %d.\n", bee_id, entrance_number);
                    		hive->bees_in_hive--;
                    		hive->bees[idx].outside = true;
                		}
            		}
		}

			hive->bees[alive_bees++] = hive->bees[idx];
		}
		 hive->total_bees=alive_bees;
			
		 pthread_mutex_unlock(&hive->lock);

	
	usleep(700000);

	sem_post(&hive->queen_semaphore);
	}

return NULL;

}
