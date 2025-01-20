#ifndef ENTRANCES_H
#define ENTRANCES_H

#include <pthread.h>
#include <stdbool.h>

//struktura wejścia do ula
typedef struct {
    pthread_mutex_t lock;   //mutex do synchronizacji wejścia
    pthread_cond_t cond;    //warunek na kierunek ruchu
    bool entry_direction;   //true-wejscie, false-wyjściee
    int bees_inside;        //liczba pszczół korzystających z wejścia
} Entrance;

//funkcje zarządzające wyjsciami
void init_entrance(Entrance* entrance);
void destroy_entrance(Entrance* entrance);
void use_entrance(Entrance* entrance, bool direction, int bee_id);

#endif // ENTRANCES_H

