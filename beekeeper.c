#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include"beehive.h"
#include<errno.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<semaphore.h>

void* beekeeper_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;

	while (running) {
                if (pthread_mutex_lock(&hive->lock) != 0) {
                        perror("pthread_mutex_lock");
                        break;
                }

                if (hive->frame_signal == 1 || hive->frame_signal == 2) {
                        hive->event_flag = 1;
                        if (hive->frame_signal == 1) {
                                hive->max_population *= 2;
                                printf("Pszczelarz dodal ramki. Maksymalna liczba pszczol: %d\n", hive->max_population);
                        } else if (hive->frame_signal == 2) {
                                hive->max_population /= 2;
                                printf("Pszczelarz usunal ramki. Maksymalna liczba pszczol: %d\n", hive->max_population);
                }
                hive->frame_signal = 0;
                if (sem_post(&hive->event_semaphore) != 0) {
			perror("sem_post");
			break;
                }
		hive->event_flag = 0;
		}

                if (pthread_mutex_unlock(&hive->lock) != 0) {
                        perror("pthread_mutex_unlock");
                        break;
                }
                usleep(1000000);
        }
        return NULL;
}
