#include<stdio.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>

void* queen_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while (running) {
        	if (pthread_mutex_lock(&hive->lock) != 0) {
			perror("pthread_mutex_lock");
			break;
		}
		
		if(!hive->queen_alive) {
			printf("Krolowa umarla!\n");
			int new_queen_found = 0;
			for (int i=1; i<hive->total_bees; i++) {
				if (hive->bees[i].type == 'W') {
					hive->bees[i].type = 'Q';
					hive->queen_alive = 1;
					hive->bees[i].age = 0;
					printf("Nowa krolowa: pszczola %d\n", i);
					new_queen_found = 1;
					break;
				}
			}
			if (!new_queen_found) {
				printf("Brak robotnic do wyboru nowej krolowej. Ul umiera!\n");
				pthread_mutex_unlock(&hive->lock);
				break;
			}
		}

        	if (hive->total_bees < hive->max_population) {
		int eggs = (rand() % 5) + 1;
		for(int i=0;i<eggs && hive->total_bees<hive->max_population;i++) {
            		hive->bees[hive->total_bees].type = 'W';
            		hive->bees[hive->total_bees].age = 0;
            		hive->bees[hive->total_bees].visits = 0;
			hive->bees[hive->total_bees].Ti = 0;
            		hive->total_bees++;
		}
            	printf("Krolowa zlozyla %d jaj", eggs);
		}

		hive->bees[0].age++;
		if (hive->bees[0].age>hive->queen_lifespan) {
			hive->queen_alive=0;

		}

        	if (pthread_mutex_unlock(&hive->lock) != 0) {
			perror("pthread_mutex_unlock");
			break;
		}

        	usleep(1000000);
    	}

    return NULL;
}
