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
		if (semop(sem_id, &sb, 1) == -1) {
			perror("Błąd podczas pobierania semafora w worker_process");
			exit(EXIT_FAILURE);
		}

		for (int i=1;i<hive->total_bees;i++) {
			if (hive->bees[i].type == 'W' && hive->bees[i].age < 10) {
				if (hive->bees[i].outside) {
					use_entrance(&hive->entrance1, true, hive->bees[i].id);
					hive->bees[i].outside = false;
					hive->bees_in_hive++;
				} else {
				use_entrance(&hive->entrance2, false, hive->bees[i].id);
				hive->bees[i].outside = true;
				hive->bees_in_hive--;
			}

			hive->bees[i].visits++;
			hive->bees[i].age++;
			if (hive->bees[i].visits > 5) {
				printf("Pszczoła %d umiera z powodu starości.\n" , hive->bees[i].id);
				hive->bees[i].Ti = 0;
			}
		}

	}
	
	sb.sem_op = 1;
	if (semop(sem_id, &sb, 1) == -1) {
		perror("Błąd podczas zwalniania semafora w worker_process");
		exit(EXIT_FAILURE);
	}

	sleep(1);

}

}
