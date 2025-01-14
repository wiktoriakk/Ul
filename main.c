#include<stdio.h>
#include<stdlib.h>
#include"beehive.h"
#include<pthread.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include"entrances.h"
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<semaphore.h>

//zmienne globalne
Beehive* hive;
int running = 1;
Entrance entrance1;
Entrance entrance2;

void handle_error(const char* message) {
        perror(message);
        exit(EXIT_FAILURE);
}

void signal_handler(int signo) {
        if (signo == SIGUSR1) {
                printf("Otrzymano sygnal SIGUSR1: Dodanie ramek.\n");
                hive->frame_signal = 1;
        } else if (signo == SIGUSR2) {
                printf("Otrzymano sygnal SIGUSR2: Usuniecie ramek.\n");
                hive->frame_signal = 2;
        } else if (signo == SIGINT) {
                running = 0;
        }
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0; // Ustawienie domyślnych flag
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        handle_error("sigaction SIGUSR1");
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        handle_error("sigaction SIGUSR2");
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        handle_error("sigaction SIGINT");
    }
}

void validate_input(int initial_bees, int max_population, int max_bees_in_hive) {
        if (initial_bees <= 0 || max_population <= 0 || max_bees_in_hive <= 0) {
                fprintf(stderr, "Blad: Wszystkie wartosci musza byc dodatnie.\n");
                exit(EXIT_FAILURE);
        }
        if (max_bees_in_hive>max_population) {
                fprintf(stderr, "Blad: Maksymalna liczba pszczol w ulu nie moze przekraczac maksymalnej populacji.\n");
                exit(EXIT_FAILURE);
        }
        if (max_bees_in_hive >= (initial_bees / 2)) {
                fprintf(stderr, "Blad: Maksymalna liczba pszczol w ulu musi byc mniejsza niz polowa poczatkowej liczby pszczol.\n");
                exit(EXIT_FAILURE);
        }
}




int main() {
        int initial_bees = 50;
        int max_population = 100;
        int max_bees_in_hive = 24;
        int queen_lifespan = 10;
        int worker_lifespan = 5;

        validate_input(initial_bees, max_population, max_bees_in_hive);

        hive = malloc(sizeof(Beehive));
        if (!hive) {
                handle_error("malloc");
        }

        hive->bees = malloc(max_population * sizeof(Bee));
        if (!hive->bees) {
                handle_error("malloc");
        }

        hive->total_bees = initial_bees;
        hive->max_population = max_population;
        hive->max_bees_in_hive = max_bees_in_hive;
        hive->queen_lifespan = queen_lifespan;
        hive->worker_lifespan = worker_lifespan;
        hive->bees_in_hive = 15;
        hive->queen_alive = 1;
        hive->frame_signal = 0;

        srand(time(NULL));
        for (int i=0; i<initial_bees; i++) {
		hive->bees[i].id = i;
                hive->bees[i].type = 'W';
                hive->bees[i].age =0;
                hive->bees[i].visits = 0;
                hive->bees[i].Ti = (rand() % 5 + 1)* 1000;
		hive->bees[i].outside = false;

    		if (i < max_bees_in_hive) {
        		hive->bees[i].outside = false; // W ulu
    		} else {
        		hive->bees[i].outside = true;  // Poza ulem
    			}
	}

        hive->bees[0].type = 'Q'; //pierwsza pszczola to krolowa
	hive->bees[0].outside = false; //krolowa zawsze w ulu

        if (pthread_mutex_init(&hive->lock, NULL) != 0) {
                handle_error("pthread_mutex_init");
        }
        if (sem_init(&hive->event_semaphore, 0, 0) != 0) {
                handle_error("sem_init");
        }
	sem_init(&hive->queen_semaphore, 0, 1);
	sem_init(&hive->worker_semaphore, 0, 1);

	init_entrance(&entrance1);
    	init_entrance(&entrance2);
        
	setup_signal_handlers();
	
	
        pthread_t queen, workers, beekeeper, monitor;

        if (pthread_create(&queen, NULL, queen_thread, hive) != 0) {
                handle_error("pthread_create");
        }
        if (pthread_create(&workers, NULL, worker_thread, hive) != 0) {
                handle_error("pthread_create");
        }
        if (pthread_create(&beekeeper, NULL, beekeeper_thread, hive) != 0) {
                handle_error("pthread_create");
        }
        if (pthread_create(&monitor, NULL, monitor_thread, hive) != 0) {
                handle_error("pthread_create");
        }

	while(running) {
		usleep(100000);
	}

        if (pthread_join(queen, NULL) != 0) {
                handle_error("pthread_join");
        }

	if (pthread_join(workers, NULL) != 0) {
    		handle_error("pthread_join");
	}
	if (pthread_join(beekeeper, NULL) != 0) {
    		handle_error("pthread_join");
	}
	if (pthread_join(monitor, NULL) != 0) {
    		handle_error("pthread_join");
	}


        printf("\n--- Podsumowanie ---\n");
        printf("Calkowita liczba pszczol: %d\n", hive->total_bees);
        printf("Krolowa: %s\n", hive->queen_alive ? "zyje" : "umarla");
        printf("Pszczoly w ulu: %d\n", hive->bees_in_hive);

        if (pthread_mutex_destroy(&hive->lock) != 0) {
                handle_error("pthread_mutex_destroy");
        }
	
	if (sem_destroy(&hive->event_semaphore) != 0) {
                handle_error("sem_destroy");
        }

	
	sem_destroy(&hive->queen_semaphore);
	sem_destroy(&hive->worker_semaphore);
	
	destroy_entrance(&entrance1);
	destroy_entrance(&entrance2);

        free(hive->bees);
        free(hive);

        return 0;
}
