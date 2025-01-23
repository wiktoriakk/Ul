#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"beehive.h"
#include<sys/sem.h>
#include<stdbool.h>
#include<errno.h>

extern int running;

void queen_process() {
	while (running && hive->queen_alive) { //główna pętla programu
		struct sembuf sb = {0, -1, 0};
		while (semop(sem_id, &sb, 1) == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("Błąd podczas pobierania semafora w queen_process");
				exit(EXIT_FAILURE);
			}
		}

		if (pthread_mutex_lock(&hive->mutex) != 0) {
			perror("Błąd podczas blokowania mutexu w queen_process");
            		sb.sem_op = 1;
            		if (semop(sem_id, &sb, 1) == -1) {
                		perror("Błąd podczas zwalniania semafora w queen_process");
            		}
            		exit(EXIT_FAILURE);
        	}

		if (!running) { //sprawdzenie czy program dalej działą
			if (pthread_mutex_unlock(&hive->mutex) != 0) {
				perror("Błąd podczas odblokowywania mutexa w queen_process");
			}
			sb.sem_op=1;
			if (semop(sem_id, &sb, 1) == -1) {
				perror("Błąd podczas zwalniania semfora w queen_process");
			}
			break;
		}

		//obsługa zmiany ramek
                if (hive->frame_signal == 1) { //jeśli frame_signal=1 to następuje zwiększenie liczby populacji
                        printf("Królowa zauważyła dodanie ramek. Maksymalna liczba pszczół: %d\n", hive->max_population);
                        hive->frame_signal = 0;
		} else if (hive->frame_signal == 2) { //jeśli jest frame_signal=2 następuje zmniejszenie liczby populacji
                        printf("Królowa zauważyła usunięcie ramek. Maksymalna liczba pszczół: %d\n", hive->max_population);
			hive->frame_signal = 0;  	
                }
		//sprawdzenie dostępnego miejsca w ulu (maksymalna liczba pszczół w ulu - liczba pszczół w ulu)
		int available_space = hive->max_bees_in_hive - hive->bees_in_hive;
	
		//upewnienie się ze nie ma miejsca w ulu
		if (hive->total_bees >= hive->max_population || available_space <= 0) {
		    printf("Ul osiągnął maksymalną populację lub brak miejsca. Kończenie symulacji.\n");
    			running = 0;
			
    			if (pthread_mutex_unlock(&hive->mutex) != 0) {
				perror("Błąd podczas odblokowywania mutexa w queen_process");
			}
            		sb.sem_op = 1;
            		if (semop(sem_id, &sb, 1) == -1) {
				perror("Błąd podczas zwalniania semafora w queen_process");
				exit(EXIT_FAILURE);
			}
            		break;
		}
					
		//operacja składania jaj
			if (available_space > 0) { //jeśli jest miejsce w ulu
				 int eggs = rand() % 10 + 1; //losowa liczba składanych jaj
        			    if (eggs > available_space) { //jeśli liczba jaj jest większa niż dostępne miejsce
	                	eggs = available_space; //liczba jaj jest równa dostępnej ilosci miejsca w ulu 
            		}


    					for (int i = 0; i < eggs; i++) { //dodanie nowej pszczoły
        				hive->bees[hive->total_bees].id = hive->total_bees;
        				hive->bees[hive->total_bees].type = 'W'; //typ robotnica
        				hive->bees[hive->total_bees].age = 0; //wiek 0=0
        				hive->bees[hive->total_bees].visits = 0; //wizyty w ulu=0
        				hive->bees[hive->total_bees].Ti = (rand() % 5 + 1) * 1000; //generowany jest losowy czas pobytu w ulu
        				hive->bees[hive->total_bees].outside = false; //nowa pszczoła jest w ulu
        				hive->bees[hive->total_bees].dead = false; //nowa pszczoła nie jest od razu martwa
        				hive->total_bees++; //zwiększenie liczby pszczół
    					}

					    hive->eggs_laid += eggs; //zwiększenie licznika złożonych jaj (do pliku podsumowującego symulację)
    						printf(YELLOW "Królowa złożyła %d jaj. Aktualna liczba pszczół: %d" RESET "\n", eggs, hive->total_bees);
					}	 else {
    						printf(RED "Królowa nie może złożyć jaj: brak miejsca lub osiągnięto maksymalną populację." RESET "\n");
					}
                   
		

		if (pthread_mutex_unlock(&hive->mutex) != 0) {
			perror("Błąd podczas odblokowywania mutexu w queen_process");\
			sb.sem_op = 1;
            		if (semop(sem_id, &sb, 1) == -1) {
                		perror("Błąd podczas zwalniania semafora w queen_process");
            		}
            		exit(EXIT_FAILURE);
        	}

		sb.sem_op = 1;
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd podczas zwalniania semafora w queen_process");
			exit(EXIT_FAILURE);
		}
		
		usleep(250000);
	
	}

	printf("Królowa zakończyła pracę.\n");
	fflush(stdout);
	exit(0);
}
