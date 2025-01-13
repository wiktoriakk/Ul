#ifndef BEEHIVE_H
#define BEEHIVE_H

#include<stdbool.h>
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
	bool outside; //czy pszczola jest poza ulem (true-tak, false-nie)
} Bee;

//struktura ula
typedef struct {
        Bee* bees; // wskaxnik na tablice pszczol
        int total_bees; //calkowita liczba pszczol
        int max_population; //maksymalna liczba pszczol
	int max_bees_in_hive;  //maksymalna liczba pszczol w ulu
	int queen_lifespan; //dlugosc zycia krolowej
	int worker_lifespan; //dligosc zycia robotnicy
	int bees_in_hive; //aktualna liczba pszczol w ulu
	pthread_mutex_t lock; //mutex do synchronizacji
	sem_t event_semaphore; //semafor do obslugi zdarzen
	int queen_alive; //status zycia krolowej
	int frame_signal; //sygnal zmiany ramek 0- bez zmiany, 1-dodanie ramek, 2-usuniecie ramek
	int event_flag; //flaga zdarzen 0-brak zdarzenia, 1-zmiana populacji
} Beehive;

extern int running;

//deklaracja funkcji
void validate_input(int initial_bees, int max_population, int max_bees_in_hive);
void* queen_thread(void* arg);
void* worker_thread(void* arg);
void* beekeeper_thread(void* arg);
void* monitor_thread(void* arg);

#endif
