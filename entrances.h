#ifndef ENTRANCES_H
#define ENTRANCES_H

#include <stdbool.h>

//struktura wejścia do ula
typedef struct {
	int sem_id_in;           // Identyfikator semafora dla wchodzenia
	int sem_id_out; //identyfikator semafora dla wychodzenia
    	bool entry_direction; // Kierunek ruchu (true - wejście, false - wyjście)
} Entrance;

//funkcje zarządzające wyjsciami
void init_entrance(Entrance* entrance);
void destroy_entrance(Entrance* entrance);
void use_entrance(Entrance* entrance, bool direction, int bee_id);

void init_global_semaphore();
void destroy_global_semaphore();

#endif // ENTRANCES_H

