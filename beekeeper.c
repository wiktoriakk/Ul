#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"beehive.h"
#include<sys/sem.h>

void beekeeper_process() {
	while (running) {
                struct sembuf sb = {0, -1, 0};
		semop(sem_id, &sb, 1);

		if (hive->frame_signal == 1) {
			hive->max_population *= 2;
                        printf("Pszczelarz dodał ramki. Maksymalna liczba pszczół: %d\n", hive->max_population);
			hive->frame_signal = 0;
                        } else if (hive->frame_signal == 2) {
                                hive->max_population /= 2;
                                printf("Pszczelarz usunął ramki. Maksymalna liczba pszczół: %d\n", hive->max_population);
                		hive->frame_signal = 0;
			}
	sb.sem_op = 1;
	semop(sem_id, &sb, 1);

	sleep(1);
	
	}
}
