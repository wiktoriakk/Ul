#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* monitor_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;
	
	while(1) {
		pthread_mutex_lock(&hive->lock);

		printf("\n--- Stan ula ---\n");
		printf("Krolowa: %s\n", hive->queen_alive ? "zyje" : "umarla");
		printf("Calkowita liczba pszczol: %d\n", hive->total_bees);
		printf("Pszczoly w ulu: %d\n", hive->bees_in_hive);

		pthread_mutex_unlock(&hive->lock);
		usleep(5000000); 
	}

return NULL;
}
