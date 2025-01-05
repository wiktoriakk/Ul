#include<stdio.h>
#include<unistd.h>
#include"beehive.h"

void* beekeeper_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;

	while (1) {
		pthread_mutex_lock(&hive->lock);

		if (hive->frame_signal == 1) {
			hive->max_population *= 2;
			hive->frame_signal = 0;
			printf("Pszczelarz dodal ramki. Maksymalna liczba pszczol: %d\n", hive->max_population);
		} else if (hive->frame_signal == 2) {
			hive->max_population /= 2;
			hive->frame_signal = 0;
			printf("Pszczelarz usunal ramki. Maksymalna liczba pszczol: %d\n", hive->max_population);
		}
			
		pthread_mutex_unlock(&hive->lock);
		usleep(1000000);
	}
	return NULL;
}
