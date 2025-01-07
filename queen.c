#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* queen_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while (1) {
        	pthread_mutex_lock(&hive->lock);
		
		if(!hive->queen_alive) {
			printf("Krolowa umarla!\n");
			for (int i=1; i<hive->total_bees; i++) {
				if (hive->bees[i].type == 'W') {
					hive-bees[i].type = 'Q';
					hive->queen_alive =1;
					hive->bees[i].age = 0;
					printf("Nowa krolowa zostala wybrana: pszczola %d\n", i);
					break;
				}
			}
			if (!hive->queen_alive) {
				printf("Nie ma dostepnych robotnic do wyboru nowej krolowej!\n");
				pthread_mutex_unlock(&hive->lock);
				break;
			}
		}

        	if (hive->total_bees < hive->max_population) {
		int eggs = 5;
		for(int i=0;i<eggs && hive->total_bees<hive->max_population;i++) {
            		hive->bees[hive->total_bees].type = 'W';
            		hive->bees[hive->total_bees].age = 0;
            		hive->bees[hive->total_bees].visits = 0;
            		hive->total_bees++;
		}
            	printf("Krolowa sklada 5 jaj. Liczba pszczol: %d\n", hive->total_bees);
		}

		hive->bees[0].age++;
		if (hive->bees[0].age>hive->queen_lifespan) {
			hive->queen_alive=0;

		}

        	pthread_mutex_unlock(&hive->lock);
        	usleep(1000000);
    	}

    return NULL;
}
