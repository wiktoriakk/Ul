#ifndef UL_H
#define UL_H
#include<pthread.h>
#define THREAD_RUNNING 1
#define THREAD_SLEEPING 2
#define THREAD_TERMINATED 3

typedef struct {
	char type;
	int age;
	int visits;
} Bee; 

typedef struct {
	Bee* bees;
	int total_bees;
	int max_population;
	int queen_life;
	int worker_life;
	int max_bees_in_beehive;
	int bees_in_beehive;
	pthread_mutex_t lock;
	int queen_alive;
} Beehive;
