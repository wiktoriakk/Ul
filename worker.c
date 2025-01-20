#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>
#include<stdbool.h>
#include<sys/sem.h>
#include"entrances.h"

extern Entrance entrance1;
extern Entrance entrance2;
extern int running;

void worker_process() {
	while(running) {
		struct sembuf sb = {0, -1, 0};
		while (semop(sem_id, &sb, 1) == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("Błąd podczas pobierania semafora w worker_process");
				exit(EXIT_FAILURE);
			}
		}
		
		pthread_mutex_lock(&hive->mutex);
		
		//sprawdzenie liczby żywych pszczół
		int alive_workers=0;
		for (int i=1;i<hive->total_bees;i++) {
			if(!hive->bees[i].dead) {
				alive_workers++;
			}
		}

		if (alive_workers == 0) {
			printf("Brak żywych pszczół. Robotnice kończą pracę.\n");
			running=0;
			pthread_mutex_unlock(&hive->mutex);
			sb.sem_op=1;
			if (semop(sem_id, &sb, 1) == -1) {
				perror("Błąd podczas zwalniania semafora w worker_process");
				exit(EXIT_FAILURE);
			}
			break;
		}

		//iteracja przez wszystkie pszczoły
		for (int i=1;i<hive->total_bees;i++) {
			Bee *current_bee = &hive->bees[i];

			if (current_bee->dead) {
				printf("Pszczoła %d jest martwa i zostaje pominięta.\n", current_bee->id);
				continue;
			}

			//aktualizacja stanu pszczoły
			if (current_bee->type == 'W' && current_bee->age < 10) {
				if (current_bee->outside) {
					if (hive->bees_in_hive < hive->max_bees_in_hive) {
						//wejście pszczoły do ula
						use_entrance(&hive->entrance1, true, current_bee->id);
						hive->bees_in_hive++;
						current_bee->outside=false;
						current_bee->Ti = (rand() % 5 + 5)*1000;
					} else {
						printf("Pszczoła %d nie może wejść: maksymalna liczba pszczół w ulu osiągnięta.\n", current_bee->id);
					}
				} else {
					//czas pobytu pszczoły w ulu
					current_bee->Ti -= 500;
				
					if (current_bee->Ti <= 0) {
						//wyjście pszczoły z ula
						use_entrance(&hive->entrance2, false, current_bee->id);
						hive->bees_in_hive--;
						current_bee->outside = true;
					}	
				}

			//aktualizacja odwiedzin ula i wieku pszczoły
			current_bee->visits++;
			current_bee->age++;
			
			//sprawdzenie czy pszczoła zmarła z powodu wielu wizyt
			if (current_bee->visits > 3) {
				printf("Pszczoła %d umiera z powodu starości.\n" , current_bee->id);
				current_bee->dead=true;
				hive->bees_in_hive--;
			}
		}

	}

	pthread_mutex_unlock(&hive->mutex);
	
	sb.sem_op = 1;
	if (semop(sem_id, &sb, 1) == -1) {
		perror("Błąd podczas zwalniania semafora w worker_process");
		exit(EXIT_FAILURE);
	}

	sleep(1);
	
	}

	printf("Robotnice zakończyły pracę.\n");
}
