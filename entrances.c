#include "entrances.h"
#include <stdio.h>
#include <unistd.h>

// Inicjalizuje wejście do ula
void init_entrance(Entrance* entrance) {
    pthread_mutex_init(&entrance->lock, NULL);
    pthread_cond_init(&entrance->cond, NULL);
    entrance->entry_direction = true; // Domyślny kierunek: wejście
    entrance->bees_inside = 0;
}

// Zwalnia zasoby związane z wejściem do ula
void destroy_entrance(Entrance* entrance) {
    pthread_mutex_destroy(&entrance->lock);
    pthread_cond_destroy(&entrance->cond);
}

// Pszczoła korzysta z wejścia do ula
void use_entrance(Entrance* entrance, bool direction, int bee_id) {
    pthread_mutex_lock(&entrance->lock);

       if (direction == false) { // Jeśli chodzi o wyjście
        // Pszczoła królowa nie może wyjść z ula
		printf("Królowa nie opuszcza ula!\n");
        	pthread_mutex_unlock(&entrance->lock);
        	return;
   	}
// Jeśli kierunek jest różny od aktualnego, pszczoła czeka
    while (entrance->bees_inside > 0 && entrance->entry_direction != direction) {        
        pthread_cond_wait(&entrance->cond, &entrance->lock);
    }
    // Ustawia kierunek wejścia i zwiększa liczbę pszczół w wejściu
    entrance->entry_direction = direction;
    entrance->bees_inside++;
    printf("Pszczoła %d %s przez wejście\n", bee_id, direction ? "wchodzi" : "wychodzi");

    pthread_mutex_unlock(&entrance->lock);

    // Symuluje czas przejścia przez wejście
    usleep(500000);

    pthread_mutex_lock(&entrance->lock);

    // Zmniejsza liczbę pszczół w wejściu
    entrance->bees_inside--;
    if (entrance->bees_inside == 0) {
        pthread_cond_broadcast(&entrance->cond); // Powiadamia inne pszczoły
    }

    pthread_mutex_unlock(&entrance->lock);
}

