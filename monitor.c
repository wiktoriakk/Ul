#include<stdio.h>
#include<sys/sem.h>
#include<stdlib.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>


void monitor_process() {
	FILE *log_file = fopen("hive_status.log", "w");
	if (log_file == NULL) {
	perror("Nie można otworzyć pliku do zapisu");
	exit(EXIT_FAILURE);
	}
	
	while(running) {
        	struct sembuf sb = {0, -1, 0};
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd podczas pobierania semafora w monitor_process");
			exit(EXIT_FAILURE);
		}

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

		sb.sem_op = 1;
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd podczas zwalniania semafora w monitor_process");
			exit(EXIT_FAILURE);
		}
	
		sleep(5);
	}

	if (fclose(log_file) != 0) {
		perror("Nie można zamknąć pliku do zapisu");
	}
}
