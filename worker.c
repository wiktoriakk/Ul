#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include"main.c"

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;
	while(hive->queen_alive) {
		 pthread_mutex_lock(&hive->lock);

        	for (int i = 0; i < hive->total_bees; i++) {
            		hive->bees[i].age++;
            		if (hive->bees[i].age > 10) {
				printf("Pszczola %d umarla z powodu starosci.\n", i);
				hive->bees[i]=hive->bees[hive->total_bees-1];
				hive->total_bees--;
			
			}
		}

		pthread_mutex_unlock(&hive->lock);
		sleep(1);
	}

return NULL;

}
