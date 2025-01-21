#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include "beehive.h"
#include"entrances.h"
#include<errno.h>
#include<pthread.h>

//zmienne globalne
int running = 1;
int shm_id;
int sem_id;
Beehive *hive;
pid_t pids[4];

//funkcja czyszcząca - zwalniająca zasoby
void cleanup() {
	printf("Rozpoczęcie czyszczenia zasobów...\n");

	printf("Niszczenie wejść..\n");
 	destroy_entrance(&hive->entrance1);
        destroy_entrance(&hive->entrance2);

	printf("Niszczenie mutexa..\n");
	if (pthread_mutex_destroy(&hive->mutex) != 0) {
                perror("Błąd niszczenia mutexa");
        }

	printf("Odłączanie pamięci współdzielonej...\n");
	if (shmdt(hive) == -1) {
		perror("Błąd podczas odłączania pamięci współdzielonej");
	}
	
	printf("Usuwanie pamięci współdzielonej...\n");
	if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
		perror("Błąd podczas usuwania pamieci współdzielonej");
	}
	
	printf("Usuwanie semafora...\n");
	if (semctl(sem_id, 0, IPC_RMID) == -1) {
		perror("Błąd podczas usuwania semafora");
	}

	printf("Zasoby zostały zwolnione.\n");
}

//obsługa sygnałów
void signal_handler(int signo) {
        if (signo == SIGUSR1) {
                printf("Otrzymano sygnał SIGUSR1: Dodanie ramek.\n");
                hive->frame_signal = 1;
        } else if (signo == SIGUSR2) {
                printf("Otrzymano sygnał SIGUSR2: Usunięcie ramek.\n");
                hive->frame_signal = 2;
        } else if (signo == SIGINT) {
		printf("Otrzymano sygnał SIGINT: Zatrzymanie symulacji.\n");
                running = 0;
        }
}


//funkcja walidująca niepoprawne, początkowe wartości ula 
void validate_hive(Beehive *hive) {
        if (hive->total_bees <= 0 || hive->max_population <= 0 || hive->max_bees_in_hive <= 0) {
                fprintf(stderr, "Błąd: Wszystkie wartości muszą być dodatnie.\n");
		cleanup(); 
               	exit(EXIT_FAILURE);
        }
        if (hive->max_bees_in_hive>hive->max_population) {
                fprintf(stderr, "Błąd: Maksymalna liczba pszczół w ulu nie może przekraczać maksymalnej populacji.\n");
                cleanup();
		exit(EXIT_FAILURE);
        }
        if (hive->max_bees_in_hive >= (hive->total_bees / 2)) {
                fprintf(stderr, "Błąd: Maksymalna liczba pszczół w ulu musi być mniejsza niż połowa początkowej liczby pszczół.\n");
                cleanup();
		exit(EXIT_FAILURE);
        }
}

//funkcja uruchamiająca procesy
pid_t start_process(void (*process_func)()) {
	pid_t pid = fork();
	if (pid == 0) {
		process_func();	
		exit(0);
	} else if (pid<0) {
		perror("Błąd podczas tworzenia procesu");
		cleanup();
		exit(EXIT_FAILURE);
	}
	return pid;
}


int main() {
	shm_id = shmget(IPC_PRIVATE, sizeof(Beehive), IPC_CREAT | 0666);
	if (shm_id < 0) {
		perror("Błąd podczas tworzenia pamięci współdzielonej");
		exit(EXIT_FAILURE);
	}

	hive=(Beehive *)shmat(shm_id, NULL, 0);
	if (hive==(void *)-1) {
		perror("Błąd podczas dołączania pamięci współdzielonej");
		cleanup();
		exit(EXIT_FAILURE);
	}

	//inicjalizacja ula
        hive->total_bees = 10;
        hive->max_population = 20;
        hive->max_bees_in_hive = 4;
        hive->bees_in_hive = 3;
        hive->queen_alive = 1;
        hive->frame_signal = 0;
	hive->bees_entered=0;
	hive->bees_exited=0;
	hive->eggs_laid=0;

	validate_hive(hive);

	//inicjalizacja pszczół
        for (int i=0; i<hive->total_bees; i++) {
		hive->bees[i].id = i;
                hive->bees[i].type = (i==0) ? 'Q' : 'W';
                hive->bees[i].age = 0;
                hive->bees[i].visits = 0;
                hive->bees[i].Ti = (rand() % 1 + 1)* 1000;
		hive->bees[i].outside = (i >= hive->bees_in_hive);
		hive->bees[i].dead=false;
	}
       
	//inicjalizacja wejść
	init_entrance(&hive->entrance1);
    	init_entrance(&hive->entrance2);
        
	sem_id=semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (sem_id < 0) {
		perror("Błąd podczas tworzenia semafora");
		cleanup();
		exit(EXIT_FAILURE);
	}
	if (semctl(sem_id, 0, SETVAL, 1) == -1) {
		perror("Błąd podczas ustawiania wartości semafora");
		cleanup();
		exit(EXIT_FAILURE);
	}

	if (pthread_mutex_init(&hive->mutex, NULL) != 0) {
		perror("Błąd podczas inicjalizacji mutexa");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
		perror("Błąd podczas ustawiania obsługi SIGUSR1");
		cleanup();
		exit(EXIT_FAILURE);
	}

	if (signal(SIGUSR2, signal_handler) == SIG_ERR) {
		perror("Błąd podczas ustawiania obsługi SIGUSR2");
		cleanup();
		exit(EXIT_FAILURE);
	}
	
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		perror("Błąd podczas ustawiania obsługi SIGINT");
		cleanup();
		exit(EXIT_FAILURE);
	}

	pids[0]=start_process(queen_process);
	pids[1]=start_process(worker_process);
	pids[2]=start_process(beekeeper_process);
	pids[3]=start_process(monitor_process);

	int terminate_monitoring=0;

	while(running) {

		int alive_bees=0;
		for (int i=0; i<hive->total_bees;i++) {
			if(!hive->bees[i].dead) {
				alive_bees++;
			}
		}

		if (alive_bees==0) {
			printf("Wszystkie pszczoły umarły. Kończenie symulacji.\n");
			terminate_monitoring=1;
		}

		if (hive->queen_alive == 0) {
			printf("Królowa umarła. Kończenie symulacji.\n");
			terminate_monitoring=1;
		}


		if (hive->bees_in_hive == 0) {
			printf("Ul zakończył pracę.\n");
			terminate_monitoring=1;
		}

		if (terminate_monitoring) {
			printf("Czekanie na zakończenie procesu monitotowania...\n");
			running=0;
			sleep(2);
		}
		sleep(1);
	}
	
	printf("Zatrzymywanie procesów potomnych...\n");
	for (int i = 0; i < 4; i++) {
    		if (kill(pids[i], SIGTERM) == -1) {
        		perror("Błąd podczas wysyłania SIGTERM do procesu potomnego");
    		}
	}

	int status;
	for (int i = 0; i < 4; i++) {
    		if (waitpid(pids[i], &status, 0) == -1) {
        		perror("Błąd podczas oczekiwania na zakończenie procesu potomnego");
    	} else {
        	printf("Proces potomny PID %d zakończony. Kod wyjścia: %d\n", pids[i], WEXITSTATUS(status));
    		}
	}
	
	cleanup();
	printf("Symulacja została zakończona.\n");
	
	return 0;
}
