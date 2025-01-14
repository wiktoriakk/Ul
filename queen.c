#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"beehive.h"
#include<errno.h>
#include"entrances.h"
#include<semaphore.h>

void* queen_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while (running) {
		printf("Królowa czeka na semafor...\n");
		sem_wait(&hive->queen_semaphore);
		printf("Królowa rozpoczęła składanie jaj.\n");

        	pthread_mutex_lock(&hive->lock);
			
                if (hive->frame_signal != 0) {
                        printf("Krolowa wstrzymuje skladanie jaj...\n");
                        pthread_mutex_unlock(&hive->lock);
                        while (hive->frame_signal != 0) {
                                usleep(100000);
                        }
                        printf("Krolowa wznawia skladanie jaj.\n");
			
			pthread_mutex_lock(&hive->lock);  	
                }

                if(!hive->queen_alive) {
                        printf("Krolowa umarla!\n");
			int new_queen_found = 0;
                        for (int i=1; i<hive->total_bees; i++) {
                                if (hive->bees[i].type == 'W') {
                                        hive->bees[i].type = 'Q';
                                        hive->queen_alive = 1;
                                        hive->bees[i].age = 0;
                                        printf("Nowa krolowa: pszczola %d\n", hive->bees[i].id);
					new_queen_found = 1;
                                        break;
                                }
                        }
                        if (!new_queen_found) {
                                printf("Brak robotnic do wyboru nowej krolowej. Ul umiera!\n");
				running = 0;
				pthread_mutex_unlock(&hive->lock);
                                break;
                        }
                }
		
		int available_space = hive->max_bees_in_hive - hive->bees_in_hive;
                if (available_space <= 0) {
                        printf("Krolowa nie moze zlozyc jaj: brak miejsca w ulu\n");        
		} else {
                        int eggs = (rand() % 5) + 1;
                        eggs = (eggs > available_space) ? available_space : eggs;

                        for(int i=0;i<eggs && hive->total_bees<hive->max_population;i++) {
				hive->bees[hive->total_bees].id = hive->total_bees;
                                hive->bees[hive->total_bees].type = 'W';
                                hive->bees[hive->total_bees].age = 0;
                                hive->bees[hive->total_bees].visits = 0;
                                hive->bees[hive->total_bees].Ti = 0;
				hive->bees[hive->total_bees].outside = false;

				hive->bees_in_hive++;
                                hive->total_bees++;
                        }
		
                        printf("Krolowa zlozyla %d jaj.\n", eggs);
                }

                hive->bees[0].age++;
                if (hive->bees[0].age>hive->queen_lifespan) {
                        hive->queen_alive=0;

                }

	        pthread_mutex_unlock(&hive->lock);
		
        usleep(1000000); // Królowa odpoczywa przez 1 sekunde
	    
	sem_post(&hive->worker_semaphore);    

	}

    return NULL;
}
