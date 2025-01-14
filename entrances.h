#ifndef ENTRANCES_H
#define ENTRANCES_H

#include <pthread.h>
#include <stdbool.h>

// Struktura wejścia do ula
typedef struct {
    pthread_mutex_t lock;   // Mutex do synchronizacji wejścia
    pthread_cond_t cond;    // Warunek na kierunek ruchu
    bool entry_direction;   // True = wejście, False = wyjście
    int bees_inside;        // Liczba pszczół korzystających z wejścia
} Entrance;

// Funkcje do zarządzania wejściami
void init_entrance(Entrance* entrance);
void destroy_entrance(Entrance* entrance);
void use_entrance(Entrance* entrance, bool direction, int bee_id);

#endif // ENTRANCES_H

