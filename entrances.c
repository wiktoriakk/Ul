#include "entrances.h"
#include <stdio.h>
#include <unistd.h>
#include<errno.h>
#include<stdlib.h>
#include"beehive.h"

//inicjalizacja wejścia do ula
void init_entrance(Entrance* entrance) {
	if (pthread_mutex_init(&entrance->lock, NULL) != 0) {
		perror("Błąd podczas inicjalizacji mutexa w init_entrance");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_init(&entrance->cond, NULL) != 0) {
		perror("Błąd podczas inicjalizacji zmiennej warunkowej w init_entrance");
		pthread_mutex_destroy(&entrance->lock);
		exit(EXIT_FAILURE);
	}
	entrance->entry_direction = true; 
    	entrance->bees_inside = 0;
}

//czyszczenie zasobów po wchodzeniu do ula
void destroy_entrance(Entrance* entrance) {
    	if (pthread_mutex_destroy(&entrance->lock) != 0) {
		perror("Błąd podczas niszczenia mutexa w destroy_entrance");
	}
    	if (pthread_cond_destroy(&entrance->cond) != 0) {
		perror("Błąd podczas niszczenia zmiennej warunkowej w destroy_entrance");
	}
}

//funkcja umożliwiająca korzystanie z wejścia do ula
void use_entrance(Entrance* entrance, bool direction, int bee_id) {
    	if (pthread_mutex_lock(&entrance->lock) != 0) {
		perror("Błąd podczas blokowania mutexa w use_entrance");
		exit(EXIT_FAILURE);
	}

       	while (entrance->bees_inside > 0 && entrance->entry_direction != direction) {
        	if (pthread_cond_wait(&entrance->cond, &entrance->lock) != 0) {
			perror("Błąd podczas oczekiwania na warunek w use_entrance");
			pthread_mutex_unlock(&entrance->lock);
			exit(EXIT_FAILURE);
		}
    	}

	if (direction && hive->bees_in_hive >= hive->max_bees_in_hive) {
		printf("\033[33mPszczoła %d nie może wejść:maksymalna liczba pszczół w ulu osiągnięta.\033[0m\n", bee_id);
		if (pthread_mutex_unlock(&entrance->lock) != 0) {
			perror("Błąd podczas odblokowywania mutexa w use_entrance");
			exit(EXIT_FAILURE);
		} 
		return;
	}

    	entrance->entry_direction = direction;
    	entrance->bees_inside++;
	if (direction) {
		hive->bees_in_hive++;
	} else {
		hive->bees_in_hive--;
	}
			
    		printf("Pszczoła %d %s przez wejście. Liczba pszczół w wejściu: %d, pszczół w ulu: %d\n", bee_id, direction ? "wchodzi" : "wychodzi", entrance->bees_inside, hive->bees_in_hive);

    	if (pthread_mutex_unlock(&entrance->lock) != 0) {
		perror("Błąd podczas odblokowywania mutexa w use_entrance");
		exit(EXIT_FAILURE);
	}

    	usleep(500000);

    	if (pthread_mutex_lock(&entrance->lock) != 0) {
		perror("Błąd podczas ponownego blokowania mutexa w use_entrance");
		exit(EXIT_FAILURE);
	}

    	entrance->bees_inside--;
    	if (entrance->bees_inside == 0) {
        	if (pthread_cond_broadcast(&entrance->cond) != 0) {
    			perror("Błąd podczas powiadamiania warunku w use_entrance");
			pthread_mutex_unlock(&entrance->lock);
			exit(EXIT_FAILURE);
		}
	}

 	if (pthread_mutex_unlock(&entrance->lock) != 0) {
		perror("Błąd podczas odblokowywania mutexa w use_entrance");
		exit(EXIT_FAILURE);
	}
}
