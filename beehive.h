#ifndef BEEHIVE_H
#define BEEHIVE_H
#include<pthread.h>
#include<semaphore.h>

#define THREAD_RUNNING 1
#define THREAD_SLEEPING 2
#define THREAD_TERMINATED 3

//struktura pszczoly
typedef struct {
        char type; //rodzaj pszczoly: 'Q' krolowa, 'W' robotnica
        int age; //wiek (zycie)
        int visits; //liczba wizyt w ulu
	int Ti; //czas pobytu w ulu
} Bee;

//struktura ula
typedef struct {
        Bee* bees;
        int total_bees; 
        int max_population;
	int max_bees_in_hive; 
	int queen_lifespan;
	int worker_lifespan;
	int bees_in_hive;
	pthread_mutex_t lock;
	int queen_alive;
	int frame_signal; //0- bez zmiany, 1-dodanie ramek, 2-usuniecie ramek
} Beehive;

extern sem_t entrance1;
extern sem_t entrance2;

void* queen_thread(void* arg);
void* worker_thread(void* arg);
void* beekeeper_thread(void* arg);
void* monitor_thread(void* arg);

#endif
