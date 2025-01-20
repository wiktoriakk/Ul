#include<stdio.h>
#include<sys/sem.h>
#include<stdlib.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>

extern int running;

void monitor_process() {
	//otwoeranie pliku do logów
	FILE *log_file = fopen("hive_status.log", "w");
	if (log_file == NULL) {
		perror("Nie można otworzyć pliku do zapisu");
		exit(EXIT_FAILURE);
	}
	
	while(running) {
        	struct sembuf sb = {0, -1, 0};
		while (semop(sem_id, &sb, 1) == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("Błąd podczas pobierania semafora w monitor_process");
				fclose(log_file);
				exit(EXIT_FAILURE);
			}
		}
		
		pthread_mutex_lock(&hive->mutex);

		if (hive->bees_in_hive == 0 && hive->queen_alive == 0) {
            		printf("Monitor: Ul zakończył pracę. Zatrzymanie procesu monitorowania.\n");
            		running = 0;
            		pthread_mutex_unlock(&hive->mutex);
			sb.sem_op=1;
			semop(sem_id, &sb, 1);
        	    break;
	        }

		/*if (!running) {
			printf("Symulacja zakończona. Proces monitorowania kończy pracę.\n");
			sb.sem_op=1;
			if (semop(sem_id, &sb, 1) == -1) {
				perror("Błąd podczas zwalniania semafora w monitor_process");
				//fclose(log_file);
				//exit(EXIT_FAILURE);
			}
			break;
		}*/

                printf("\n--- Stan ula ---\n");
                fprintf(log_file, "\n--- Stan ula ---\n");

		printf("Królowa: %s\n", hive->queen_alive ? "żyje" : "umarła");
		fprintf(log_file, "Królowa: %s\n", hive->queen_alive ? "żyje" : "umarła");
                
		printf("Całkowita liczba pszczół: %d\n", hive->total_bees);
		fprintf(log_file, "Całkowita liczba pszczół: %d\n", hive->total_bees);                

		printf("Pszczoły w ulu: %d\n", hive->bees_in_hive);
		fprintf(log_file, "Pszczoły w ulu: %d\n", hive->bees_in_hive);

		printf("Wejście 1: %d pszczół (kierunek: %s)\n",
			hive->entrance1.bees_inside,
			hive->entrance1.entry_direction ? "wejście" : "wyjście");
		fprintf(log_file, "Wejście 1: %d pszczół (kierunek: %s)\n",
                	hive->entrance1.bees_inside,
                	hive->entrance1.entry_direction ? "wejście" : "wyjście");

        	printf("Wejście 2: %d pszczół (kierunek: %s)\n",
               		hive->entrance2.bees_inside,
               		hive->entrance2.entry_direction ? "wejście" : "wyjście");
        	fprintf(log_file, "Wejście 2: %d pszczół (kierunek: %s)\n",
                	hive->entrance2.bees_inside,
                	hive->entrance2.entry_direction ? "wejście" : "wyjście");
		
		fflush(log_file);

		pthread_mutex_unlock(&hive->mutex);

		sb.sem_op = 1;
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd podczas zwalniania semafora w monitor_process");
			fclose(log_file);
			exit(EXIT_FAILURE);
		}
	
		sleep(5);
	}

	if (fclose(log_file) != 0) {
		perror("Nie można zamknąć pliku do zapisu");
	}

	printf("Proces monitorowania zakończył działanie.\n");
}
