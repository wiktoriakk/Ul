#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* monitor_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;
	
	while(1) {
		pthread_mutex_lock(&hive->lock);

		printf("\n--- Stan ula ---\n");
		printf("Liczba pszczol: %d\n", hive->total_bees);
		printf("Pszczoly w ulu: %d\n", hive->bees_in_hive);
		printf("Krolowa zyje: %s\n", hive->queen_alive ? "TAK" : "NIE");
		printf("Maksymalna liczba pszczol: %d\n", hive->max_population);

		pthread_mutex_unlock(&hive->lock);
		usleep(2000000); //raport co 2 sekundy
	}

return NULL;
}
