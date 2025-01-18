#include "entrances.h"
#include <stdio.h>
#include <unistd.h>
#include<errno.h>
#include<stdlib.h>

// Inicjalizuje wejście do ula
void init_entrance(Entrance* entrance) {
	if (pthread_mutex_init(&entrance->lock, NULL) != 0) {
		perror("Błąd podczas inicjalizacji mutexa w init_entrance");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_init(&entrance->cond, NULL) != 0) {
		perror("Błąd podczas inicjalizacji warunku w init_entrance");
		exit(EXIT_FAILURE);
	}
	entrance->entry_direction = true; // Domyślny kierunek: wejście
    	entrance->bees_inside = 0;
}

// Zwalnia zasoby związane z wejściem do ula
void destroy_entrance(Entrance* entrance) {
    	if (pthread_mutex_destroy(&entrance->lock) != 0) {
		perror("Błąd podczas niszczenia mutexa w destroy_entrance");
		exit(EXIT_FAILURE);
	}
    	if (pthread_cond_destroy(&entrance->cond) != 0) {
		perror("Błąd podczas niszczenia warunku w destroy_entrance");
		exit(EXIT_FAILURE);
	}
}

// Pszczoła korzysta z wejścia do ula
void use_entrance(Entrance* entrance, bool direction, int bee_id) {
    	if (pthread_mutex_lock(&entrance->lock) != 0) {
		perror("Błąd podczas blokowania mutexa w use_entrance");
		exit(EXIT_FAILURE);
	}

    	// Jeśli kierunek jest różny od aktualnego, pszczoła czeka
    	while (entrance->bees_inside > 0 && entrance->entry_direction != direction) {
        	if (pthread_cond_wait(&entrance->cond, &entrance->lock) != 0) {
			perror("Błąd podczas oczekiwania na warunek w use_entrance");
			pthread_mutex_unlock(&entrance->lock);
			exit(EXIT_FAILURE);
		}
    	}

    	// Ustawia kierunek wejścia i zwiększa liczbę pszczół w wejściu
    	entrance->entry_direction = direction;
    	entrance->bees_inside++;
    	printf("Pszczoła %d %s przez wejście\n", bee_id, direction ? "wchodzi" : "wychodzi");

    	if (pthread_mutex_unlock(&entrance->lock) != 0) {
		perror("Błąd podczas odblokowywania mutexa w use_entrance");
		exit(EXIT_FAILURE);
	}

    	// Symuluje czas przejścia przez wejście
    	usleep(500000);

    	if (pthread_mutex_lock(&entrance->lock) != 0) {
		perror("Błąd podczas ponownego blokowania mutexa w use_entrance");
		exit(EXIT_FAILURE);
	}

    	// Zmniejsza liczbę pszczół w wejściu
    	entrance->bees_inside--;
    	if (entrance->bees_inside == 0) {
        	if (pthread_cond_broadcast(&entrance->cond) != 0) { // Powiadamia inne pszczoły
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
