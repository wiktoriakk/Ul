#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>
#include"entrances.h"
#include<stdlib.h>


void init_entrance(Entrance* entrance) {
    if (sem_init(&entrance->access, 0, 1) != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&entrance->lock, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    entrance->current_direction = -1;
    entrance->waiting_in = 0;
    entrance->waiting_out = 0;
}



void use_entrance(Entrance* entrance, int direction) {
    pthread_mutex_lock(&entrance->lock);

    // Jeśli kierunek jest inny, pszczoła musi czekać
    if (entrance->current_direction != -1 && entrance->current_direction != direction) {
        if (direction == 1) {
            entrance->waiting_in++;
        } else {
            entrance->waiting_out++;
        }
        pthread_mutex_unlock(&entrance->lock);

        // Czekanie na zmianę kierunku
        sem_wait(&entrance->access);

        pthread_mutex_lock(&entrance->lock);
        if (direction == 1) {
            entrance->waiting_in--;
        } else {
            entrance->waiting_out--;
        }
    }

    // Aktualizacja kierunku i dostęp
    entrance->current_direction = direction;
    pthread_mutex_unlock(&entrance->lock);
}

void release_entrance(Entrance* entrance) {
    pthread_mutex_lock(&entrance->lock);

    // Sprawdzenie, czy zmiana kierunku jest konieczna
    if (entrance->waiting_in > 0 && entrance->waiting_out == 0) {
        entrance->current_direction = 1; //wchodzenie do ula
	sem_post(&entrance->access);
    } else if (entrance->waiting_out > 0 && entrance->waiting_in == 0) {
        entrance->current_direction = 0; //wychodzenie z ula
	sem_post(&entrance->access);
    } else if (entrance->waiting_in == 0 && entrance->waiting_out == 0) {
        entrance->current_direction = -1; //brak ruchu
	sem_post(&entrance->access);
    }

    // Zwolnienie dostępu
    pthread_mutex_unlock(&entrance->lock);

}

void destroy_entrance(Entrance* entrance) {
    if (sem_destroy(&entrance->access) != 0) {
        perror("sem_destroy");
    }
    if (pthread_mutex_destroy(&entrance->lock) != 0) {
        perror("pthread_mutex_destroy");
    }
}
