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

//kody ANSI dla kolorów
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"

//struktura pszczoły
typedef struct {
	int id; //identyfikator pszczoły
        char type; //rodzaj pszczoły: 'Q' krolowa, 'W' robotnica
        int age; //wiek pszczoły
        int visits; //liczba wizyt w ulu
	int Ti; //czas pobytu w ulu
	bool outside; //czy pszczoła jest poza ulem (true-tak, false-nie)
	bool dead; //czy pszczoła jest martwa
	pid_t process_id; //pid procesu reprezentującego pszczołę
} Bee;

//struktura ula
typedef struct {
        Bee bees[1000]; //tablica przechowująca informacje o pszczołach
        int total_bees; //całkowita liczba pszczół
        int max_population; //maksymalna liczba pszczół
	int max_bees_in_hive;  //maksymalna liczba pszczół w ulu
	int bees_in_hive; //aktualna liczba pszczół w ulu
	int queen_alive; //czy królowa żyje (1-żyje, 0-umarła)
	int frame_signal; //sygnał zmiany ramek (0-bez zmiany, 1-dodanie ramek, 2-usunięcie ramek)
	Entrance entrance1; //pierwsze wejście do ula
	Entrance entrance2; //drugie wejście do ula
	pthread_mutex_t mutex; //mutex do synchronizacji stanu ula
	int bees_entered; //liczba pszczół, które weszły do ula
	int bees_exited; //liczba psczół, które wyszły z ula
	int eggs_laid; //liczba jaj złożonych przez królową
} Beehive;

//zmienne globalne
extern int running; //status symulacji (1-działa, 0-zatrzymana)
extern int shm_id; //identyfikator pamięci współdzielonej
extern int sem_id; //identyfikator semafora
extern Beehive *hive; //wskaźnik na strukturę ula w pamięci współdzielonej


//deklaracje funkcji procesów
void queen_process(); //proces królowej 
void beekeeper_process(); //proces pszczelarza
void monitor_process(); //proces monitorowania stanu ula


#endif
