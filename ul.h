#ifndef UL_H
#define UL_H

#include<pthread.h>

typedef struct {
	char type;
	int age;
	int visits;
} Bee;


typedef struct {
	Bee* bees;
	int total_bees;
	int max_population;
	int queen_lifespan;
	int worker_lifespan;
	int max_bees_in_hive;
	int bees_in_hive;
	pthread_mutex_t lock;
	int queen_alive;
} Beehive;

#endif

