#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include"worker.c"

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;
	while(hive->queen_alive) {
		sem_wait(&hive->entrance1);
		
		int bees_in_hive = 0;
        	for (int i = 0; i < hive->total_bees; i++) {
			bees_in_hive++;
			if(bees_in_hive>hive->max_bees_in_hive) {
				printf("Przegrzanie ula! Liczba pszczol: %d\n", bees_in_hive);
				break;
			}
		}
            		hive->bees[i].age++;
            		if (hive->bees[i].age > 10) {
				printf("Pszczola %d umarla z powodu starosci.\n", i);
				hive->bees[i]=hive->bees[hive->total_bees-1];
				hive->total_bees--;
			
			}
		}

		sem_post(&hive->entrance1);
		sleep(1);
	}

return NULL;

}
