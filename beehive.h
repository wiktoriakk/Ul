#ifndef BEEHIVE_H
#define BEEHIVE_H

#include<stdbool.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<time.h>
#include<pthread.h>
#include"entrances.h"

//struktura pszczoly
typedef struct {
	int id; //identyfikator pszczoly
        char type; //rodzaj pszczoly: 'Q' krolowa, 'W' robotnica
        int age; //wiek (zycie)
        int visits; //liczba wizyt w ulu
	int Ti; //czas pobytu w ulu
	bool outside; //czy pszczola jest poza ulem (true-tak, false-nie)
} Bee;

//struktura ula
typedef struct {
        Bee bees[100]; // wskaznik na tablice pszczol
        int total_bees; //calkowita liczba pszczol
        int max_population; //maksymalna liczba pszczol
	int max_bees_in_hive;  //maksymalna liczba pszczol w ulu
	int bees_in_hive; //aktualna liczba pszczol w ulu
	int queen_alive; //czy krolowa zyje
	int frame_signal; //sygnal zmiany ramek 0- bez zmiany, 1-dodanie ramek, 2-usuniecie rame
	Entrance entrance1;
	Entrance entrance2;
} Beehive;

//zmienne globalne
extern int running;
extern int shm_id;
extern int sem_id;
extern Beehive *hive;

//deklaracja funkcji
//void validate_input(int initial_bees, int max_population, int max_bees_in_hive);
void queen_process();
void worker_process();
void beekeeper_process();
void monitor_process();

#endif
