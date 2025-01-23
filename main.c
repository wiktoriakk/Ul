#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include "beehive.h"
#include "entrances.h"
#include <errno.h>
#include <pthread.h>
#include "bee.h"

//zmienne globalne
int running = 1;
int shm_id;
int sem_id;
Beehive *hive;
pid_t pids[3];

void handle_signal(int signo) {
    if (signo == SIGTERM) {
        printf("Proces (PID: %d) otrzymał sygnał zakończenia.\n", getpid());
        fflush(stdout);
        exit(0);
    }
}

//funkcja czyszcząca - zwalniająca zasoby
void cleanup() {
	printf("Rozpoczęcie czyszczenia zasobów...\n");

	printf("Niszczenie wejść..\n");
 	destroy_entrance(&hive->entrance1);
        destroy_entrance(&hive->entrance2);

   	printf("Usuwanie globalnego semafora...\n");
    	destroy_global_semaphore();	
	
	        printf("Odblokowywanie mutexa...\n");
        if (pthread_mutex_trylock(&hive->mutex) == 0) {
                pthread_mutex_unlock(&hive->mutex);
        }

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
	        
	printf("Zasoby zostały zwolnione.\n");
}

//obsługa sygnałów
void signal_handler(int signo) {
        if (signo == SIGUSR1) {
                printf("\033[32mOtrzymano sygnał SIGUSR1: Dodanie ramek.\033[0m\n");
                hive->frame_signal = 1;
        } else if (signo == SIGUSR2) {
                printf("\033[31mOtrzymano sygnał SIGUSR2: Usunięcie ramek.\033[0m\n");
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
        if (hive->max_bees_in_hive >= hive->total_bees / 2) {
                fprintf(stderr, "Błąd: Maksymalna liczba pszczół w ulu musi być mniejsza niż połowa początkowej liczby pszczół.\n");
                cleanup();
		exit(EXIT_FAILURE);
        }
}

//funkcja uruchamiająca procesy
pid_t start_process(void (*process_func)(), const char *process_name) {
	pid_t pid = fork();
	if (pid == 0) {
		signal(SIGINT, SIG_IGN);
		printf("Proces %s rozpoczął działanie (PID: %d).\n", process_name, getpid());
		process_func();	
		exit(0);
	} else if (pid<0) {
		perror("Błąd podczas tworzenia procesu");
		cleanup();
		exit(EXIT_FAILURE);
	}
	printf("Proces %s uruchomiony (PID: %d).\n", process_name, pid);
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
        hive->total_bees = 50;
       	hive->max_population = 100;
        hive->max_bees_in_hive = 24;
        hive->bees_in_hive = 1;
        hive->queen_alive = 1;
        hive->frame_signal = 0;
	hive->bees_entered=0;
	hive->bees_exited=0;
	hive->eggs_laid=0;

	validate_hive(hive);
       
	//inicjalizacja wejść
	init_entrance(&hive->entrance1);
    	init_entrance(&hive->entrance2);
        init_global_semaphore();

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

	pids[0]=start_process(queen_process, "królowa");
	pids[1]=start_process(beekeeper_process, "pszczelarz");
	pids[2]=start_process(monitor_process, "monitor");

	//inicjalizacja pszczół
        for (int i=0; i<hive->total_bees; i++) {
                hive->bees[i].id = i;
                hive->bees[i].type = (i==0) ? 'Q' : 'W';
                hive->bees[i].age = 0;
                hive->bees[i].visits = 0;
                hive->bees[i].Ti = (rand() % 3 + 1)* 1000;
                hive->bees[i].outside = (i >= hive->bees_in_hive);
                hive->bees[i].dead=false;

        if (start_bee_process(&hive->bees[i]) == -1) {
                perror("Błąd podczas tworzenia procesu pszczoły");
                cleanup();
                exit(EXIT_FAILURE);
                }

        }

	while(running) {

		int alive_bees=0;
		for (int i=0; i<hive->total_bees;i++) {
			if(!hive->bees[i].dead) {
				alive_bees++;
			}
		}

		if (alive_bees == 0 || hive->queen_alive == 0 || hive->bees_in_hive == 0) {
			printf("Ul umiera!\n");
			running=0;
		}

		sleep(1);
	}

	// Zatrzymywanie procesów pszczół
        for (int i = 0; i < hive->total_bees; i++) {
                if (kill(hive->bees[i].process_id, SIGTERM) == -1) {
                perror("Błąd podczas wysyłania SIGTERM procesu pszczoły");
                } 

	if (waitpid(hive->bees[i].process_id, NULL, 0) == -1) {
                perror("Błąd podczas oczekiwania na zakończenie procesu pszczoły");
                } else {
                        printf("Proces pszczoły %d zakończony.\n", hive->bees[i].id);
                }
       
        }	

	printf("Zatrzymywanie procesów królowej, pszczelarza i monitora...\n");
	for (int i = 0; i < 3; i++) {
    		if (kill(pids[i], SIGTERM) == -1) {
        		perror("Błąd podczas wysyłania SIGTERM do procesu potomnego");
    		}

    		if (waitpid(pids[i], NULL, 0) == -1) {
        		perror("Błąd podczas oczekiwania na zakończenie procesu potomnego");
    	} else {
        	printf("Proces potomny PID %d zakończony.\n", pids[i]);
    		}
	}

	cleanup();
	printf("Symulacja została zakończona.\n");
	
	return 0;
}
