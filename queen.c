#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"beehive.h"
#include<sys/sem.h>
#include<stdbool.h>

void queen_process() {
	while (running) {
		struct sembuf sb = {0, -1, 0};
		semop(sem_id, &sb, 1);
		
		//zmiana ramek
                if (hive->frame_signal == 1) {
                        printf("Królowa zauważyła dodanie ramek. Maksymalna liczba pszczół: %d\n", hive->max_population);
                        hive->frame_signal = 0;
		} else if (hive->frame_signal == 2) {
                        printf("Królowa zauważyła usunięcie ramek. Maksymalna liczba pszczół: %d\n", hive->max_population);
			hive->frame_signal = 0;  	
                }
		
		//logika składanie jaj lub wybór nowej królowej
                if(!hive->queen_alive) {
                        printf("Królowa umarła. Szukanie nowej królowej...\n");
                        for (int i=1; i<hive->total_bees; i++) {
                                if (hive->bees[i].type == 'W') {
                                        hive->bees[i].type = 'Q';
                                        hive->queen_alive = 1;
                                        printf("Nowa królowa: pszczoła %d\n", hive->bees[i].id);
                                        break;
                                }
                        }
		} else if (hive->bees_in_hive < hive->max_bees_in_hive) {		
                        int eggs = rand() % 5 + 1;
                        int available_space = hive->max_bees_in_hive - hive->bees_in_hive;
			if (eggs > available_space) eggs = available_space;

			hive->bees_in_hive += eggs;
                        hive->total_bees += eggs;
                        printf("Królowa złożyła %d jaj Aktualna liczba pszczół: %d\n", eggs, hive->total_bees);
                }

                sb.sem_op = 1;
		semop(sem_id, &sb, 1);
	
		sleep(1);
	
	}

}
