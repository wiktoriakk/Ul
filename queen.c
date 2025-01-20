#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"beehive.h"
#include<sys/sem.h>
#include<stdbool.h>
#include<errno.h>

extern int running;

void queen_process() {
	while (running) {
		struct sembuf sb = {0, -1, 0};
		while (semop(sem_id, &sb, 1) == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("Błąd podczas pobierania semafora w queen_process");
				exit(EXIT_FAILURE);
			}
		}

		pthread_mutex_lock(&hive->mutex);

		//zmiana ramek
                if (hive->frame_signal == 1) {
                        printf("Królowa zauważyła dodanie ramek. Maksymalna liczba pszczół: %d\n", hive->max_population);
                        hive->frame_signal = 0;
		} else if (hive->frame_signal == 2) {
                        printf("Królowa zauważyła usunięcie ramek. Maksymalna liczba pszczół: %d\n", hive->max_population);
			hive->frame_signal = 0;  	
                }
		
		//sprawdzenie liczby żywych robotnic
		int alive_workers=0;
		for(int i=1;i<hive->total_bees;i++) {
			if (!hive->bees[i].dead && hive->bees[i].type == 'W') {
				alive_workers++;
			}
		}
		if (alive_workers == 0) {
			printf("Brak robotnic. Ul umiera!\n");
			hive->queen_alive=0;
			running=0;
			pthread_mutex_unlock(&hive->mutex);
			sb.sem_op=1;
			if (semop(sem_id, &sb, 1) == -1) {
				perror("Błąd podczas zwalniania semafora w queen_process");
				exit(EXIT_FAILURE);
			}
			break;
		}
		
		//wybór nowej królowej
                if(!hive->queen_alive) {
                        printf("Królowa umarła. Szukanie nowej królowej...\n");
			bool new_queen_found= false;

                        for (int i=1; i<hive->total_bees; i++) {
                                if (hive->bees[i].type == 'W') {
                                        hive->bees[i].type = 'Q';
                                        hive->queen_alive = 1;
					hive->bees[i].age = 0;
                                        printf("Nowa królowa: pszczoła %d\n", hive->bees[i].id);
					new_queen_found=true;
                                        break;
                                }
                        }

			if (!new_queen_found) {
				printf("Brak robotnic do wyboru nowej królowej. Ul umiera!\n");
				running=0;
				sb.sem_op=1;
				if (semop(sem_id, &sb, 1) == -1) {
					perror("Błąd podczas zwalniania semafora w queen_process");
					exit(EXIT_FAILURE);
				}
				break;
			}
		//składanie jaj
		} else {
			if (hive->total_bees >= hive->max_population) {
				printf("Królowa nie może złożyć jaj: maksymalna populacja osiągnięta.\n");
				//continue;
			} else if (hive->bees_in_hive < hive->max_bees_in_hive) {
                        		int eggs = rand() % 5 + 1;
                        		int available_space = hive->max_bees_in_hive - hive->bees_in_hive;
			
					if (eggs > available_space) {
						eggs = available_space;
					}

					for (int i=0; i<eggs && hive->total_bees < hive->max_population; i++) {
						hive->bees[hive->total_bees].id=hive->total_bees;
						hive->bees[hive->total_bees].type='W';
						hive->bees[hive->total_bees].age=0;
						hive->bees[hive->total_bees].visits=0;
						hive->bees[hive->total_bees].Ti=(rand()%5+1)*1000;
						hive->bees[hive->total_bees].outside=true;
						hive->bees[hive->total_bees].dead=false;
						hive->total_bees++;
					}
                        		printf("Królowa złożyła %d jaj. Aktualna liczba pszczół: %d\n", eggs, hive->total_bees);                		
			}
                   
		}

		pthread_mutex_unlock(&hive->mutex);
                sb.sem_op = 1;
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd podczas zwalniania semafora w queen_process");
			exit(EXIT_FAILURE);
		}
		
		sleep(1);
	
	}

	printf("Królowa zakończyła pracę.\n");
}
