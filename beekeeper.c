#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"beehive.h"
#include<sys/sem.h>
#include<errno.h>

extern int running;

void beekeeper_process() {
	while (running) {
                struct sembuf sb = {0, -1, 0};
		while (semop(sem_id, &sb, 1) == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("Błąd podczas pobierania semafora w beekeeper_process");
				exit(EXIT_FAILURE);
			}
		}

		pthread_mutex_lock(&hive->mutex);
		
		//sprawdzanie running
		if (!running) {
			printf("Pszczelarz kończy działanie.\n");
			sb.sem_op=1;
			if (semop(sem_id, &sb, 1) == -1) {
				perror("Błąd podczas zwalniania semafora w beekeeper_process");
			}	
			break;
		}

		//obsługa zmiany ramek
		if (hive->frame_signal == 1) {
			hive->max_population *= 2;
                        printf("Pszczelarz dodał ramki. Maksymalna liczba pszczół: %d\n", hive->max_population);
			hive->frame_signal = 0;
                } else if (hive->frame_signal == 2) {
                                hive->max_population /= 2;
                                printf("Pszczelarz usunął ramki. Maksymalna liczba pszczół: %d\n", hive->max_population);
                		hive->frame_signal = 0;
		}

		pthread_mutex_unlock(&hive->mutex);

		sb.sem_op = 1;
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd ppdczas zwalniania semafora w beekeeper_process");
			exit(EXIT_FAILURE);
		}

		sleep(1);
	
		}

		printf("Pszczelarz zakończył pracę.\n");
}
