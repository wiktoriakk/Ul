#include<stdio.h>
#include<unistd.h>
#include"monitor.h"

void* monitor_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;
	
	while(hive->queen_alive) {
		pthread_mutex_lock(&hive->lock);
		printf("Stan ula:\n");
		printf("Krolowa: %s\n", hive->queen_alive ? "zyje" : "umarla");
		printf("Liczba pszczol: %d\n", hive->total_bees);

		pthread_mutex_unlock(&hive->lock);
		sleep(3);
	}

return NULL;
}
