#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include"beehive.h"
#include<errno.h>
#include<fcntl.h>

void* monitor_thread(void* arg) {
	Beehive* hive=(Beehive*)arg;
	
	int log_file = creat("hive_stat.log", 0644);
	if (log_file < 0) {
		perror("Blad podczas otwierania pliku hive_stat.log");
		return NULL;
	}
	
	while(running) {
        		if (pthread_mutex_lock(&hive->lock) != 0) {
			perror("Blad podczas blokowania mutexa w monitor_thread");
			break;
		}

                printf("\n--- Stan ula ---\n");
                printf("Krolowa: %s\n", hive->queen_alive ? "zyje" : "umarla");
                printf("Calkowita liczba pszczol: %d\n", hive->total_bees);
                printf("Pszczoly w ulu: %d\n", hive->bees_in_hive);

		char buffer[256];
		int len = snprintf(buffer, sizeof(buffer),
			"--- Stan ula ---\n"
			"Krolowa: %s\n"
			"Calkowita liczba pszczol: %d\n"
			"Pszczoly w ulu: %d\n\n",
			hive->queen_alive ? "zyje" : "umarla",
			hive->total_bees,
			hive->bees_in_hive);
		
		if (write(log_file, buffer, len) < 0) {
			perror("Blad podczas zapisu do pliku hive_stat.log");
		}
		
		if (!hive->queen_alive && hive->bees_in_hive == 0) {
            		printf("Brak robotnic do wyboru nowej krolowej. Ul umiera!\n");
            		running = 0; // Zakończenie działania programu
			if (pthread_mutex_unlock(&hive->lock) != 0) {
				perror("Blad podczas odblokowywania mutexa w monitor_thread.");
			}
			break;
        	}

                if (pthread_mutex_unlock(&hive->lock) != 0) {
			perror("Blad podczas odblokowywania mutexa w monitor_thread.");
			break;
		}

                usleep(5000000);
	
	}

	if (close(log_file) < 0) {
		perror("Blad podczas zamykania pliku hive_stat.log");
	}


	return NULL;
}
