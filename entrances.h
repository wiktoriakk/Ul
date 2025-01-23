#ifndef ENTRANCES_H
#define ENTRANCES_H

#include <stdbool.h>

//struktura wejścia do ula
typedef struct {
	int sem_id_in;           //identyfikator semafora dla wchodzenia
	int sem_id_out; //identyfikator semafora dla wychodzenia
    	bool entry_direction; //kierunek ruchu (true - wejście, false - wyjście)
} Entrance;

//funkcje do zarządzania wejściami
void init_entrance(Entrance* entrance);
void destroy_entrance(Entrance* entrance);
void use_entrance(Entrance* entrance, bool direction, int bee_id);

//funkcje globalnego semafora
void init_global_semaphore();
void destroy_global_semaphore();

#endif // ENTRANCES_H

