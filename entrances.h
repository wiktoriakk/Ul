#ifndef ENTRNACES_H
#define ENTRANCES_H

#include<semaphore.h>
#include<pthread.h>

typedef struct {
	sem_t access; // kontrola dostepu do wejscia
	int current_direction; //0-na zewnatrz, 1 -do ula
	int waiting_in; //liczba pszczol czekajacych na wejscie
	int waiting_out; //liczba pszczol czekajacych na wyjscie
	pthread_mutex_t lock;
} Entrance;

void use_entrance(Entrance* entrance, int direction);
void release_entrance(Entrance* entrance);
void init_entrance(Entrance* entrance);
void destroy_entrance(Entrance* entrance);

#endif // ENTRANCES_H
