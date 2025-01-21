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

		if (pthread_mutex_lock(&hive->mutex) != 0) {
			perror("Błąd podczas blokowaniua mutexu w beekeeper_process");
			sb.sem_op = 1;
            		if (semop(sem_id, &sb, 1) == -1) {
                		perror("Błąd podczas zwalniania semafora w beekeeper_process");
            		}
            		exit(EXIT_FAILURE);
        	}

		
		//sprawdzanie running
		if (!running) {
			printf("Pszczelarz kończy działanie.\n");
			sb.sem_op=1;
			if (semop(sem_id, &sb, 1) == -1) {
				perror("Błąd podczas zwalniania semafora w beekeeper_process");
			}
			if (pthread_mutex_unlock(&hive->mutex) != 0) {
                		perror("Błąd podczas odblokowywania mutexu w beekeeper_process");
            		}	
			break;
		}

		//obsługa zmiany ramek
		if (hive->frame_signal == 1) {
			hive->max_population *= 2;
			hive->max_bees_in_hive = hive->max_population / 2;
                        printf("Pszczelarz dodał ramki. Maksymalna liczba pszczół: %d\n", hive->max_population);
			hive->frame_signal = 0;
                } else if (hive->frame_signal == 2) {
                        hive->max_population /= 2;
			hive->max_bees_in_hive = hive->max_population / 2;
                        printf("Pszczelarz usunął ramki. Maksymalna liczba pszczół: %d\n", hive->max_population);
	
			if (hive->bees_in_hive > hive->max_bees_in_hive) {
        		        int excess_bees = hive->bees_in_hive - hive->max_bees_in_hive;
                		printf("Zbyt wiele pszczół w ulu po zmniejszeniu ramek. Usuwanie %d nadmiarowych pszczół.\n", excess_bees);
	
        	        for (int i = 0; i < hive->total_bees && excess_bees > 0; i++) {
                	    if (!hive->bees[i].dead && !hive->bees[i].outside) {
                        	hive->bees[i].outside = true;
                        	hive->bees_in_hive--;
                        	excess_bees--;
                        	printf("Pszczoła %d została zmuszona do opuszczenia ula.\n", hive->bees[i].id);
                    		}
                	}
            	}

                	hive->frame_signal = 0;
		

		}

		if (pthread_mutex_unlock(&hive->mutex) != 0) {
        		perror("Błąd podczas odblokowywania mutexu w beekeeper_process");
            		sb.sem_op = 1;
            		if (semop(sem_id, &sb, 1) == -1) {
                		perror("Błąd podczas zwalniania semafora w beekeeper_process");
            		}
            		exit(EXIT_FAILURE);
        	}

		sb.sem_op = 1;
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd ppdczas zwalniania semafora w beekeeper_process");
			exit(EXIT_FAILURE);
		}

		sleep(1);
	
		}

		printf("Pszczelarz zakończył pracę.\n");
}
