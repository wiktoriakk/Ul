#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>
#include<stdbool.h>
#include"entrances.h"

extern Entrance entrance1;
extern Entrance entrance2;
extern int running;

void* worker_thread(void* arg) {
	Beehive* hive = (Beehive*)arg;

	while(running) {
		//losuj wejscie do ula
		Entrance* entrance = (rand() % 2 ==0) ? &entrance1 : &entrance2;
	
	        pthread_mutex_lock(&hive->lock);
		
		//aktualizacja danych pszczol		
		int alive_bees = 0;

		for (int i=0;i<hive->total_bees;i++) {
			hive->bees[i].age++;
 
			//spr czy pszczola osiagnela swoj maksymalny wiek
			if (hive->bees[i].type == 'W') {
				hive->bees[i].visits++;
				if(hive->bees[i].visits > hive->worker_lifespan) {
				printf("Pszczola %d umarla z powodu starosci.\n", i);
				continue;
			}
			
			if (hive->bees[i].Ti == 0) {
				hive->bees[i].Ti = (rand() % 5 + 5) * 1000;
			}
			//zmniejszenie czasu wewnatrz ila
			if (hive->bees[i].Ti>0) {
				hive->bees[i].Ti -= 500;
			}
			
			// Wybierz kierunek dla pszczoły
            		int direction = hive->bees[i].outside ? 1 : 0;
	
			// Użyj wejścia
            		pthread_mutex_unlock(&hive->lock);  // Odblokuj przed użyciem wejścia
            		use_entrance(entrance, direction);
            		pthread_mutex_lock(&hive->lock);    // Zablokuj ponownie po użyciu wejścia
			
			if (direction == 1 && hive->bees[i].outside) {
				printf("Pszczola %d wchodzi do ula przez wejscie %s\n", i, (entrance == &entrance1) ? "1" : "2");
				hive->bees_in_hive++;
				hive->bees[i].outside = false;
			} else if (direction == 0 && !hive->bees[i].outside) {
				printf("Pszczola %d opuszcza ul przez wejscie %s\n", i, (entrance == &entrance1) ? "1" : "2");
				hive->bees_in_hive--;
				hive->bees[i].outside = true;
			} else {
				printf("Ul jest pelny.\n");
			}
			
		}	
			hive->bees[alive_bees++] = hive->bees[i];
			
		}

		hive->total_bees=alive_bees;
	
	pthread_mutex_unlock(&hive->lock);
		
	release_entrance(entrance);

	usleep(700000);

	}

return NULL;

}
