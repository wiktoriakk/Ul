#include "beehive.h"
#include<string.h>

int shm_id = -1;

// Funkcja do inicjalizacji pamięci współdzielonej
Beehive* setup_shared_memory(size_t size) {
    // Tworzenie pamięci współdzielonej
    shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Błąd podczas tworzenia pamięci współdzielonej (shmget)");
        exit(EXIT_FAILURE);
    }

    // Dołączanie pamięci współdzielonej
    Beehive* hive = (Beehive*)shmat(shm_id, NULL, 0);
    if (hive == (void*)-1) {
        perror("Błąd podczas dołączania pamięci współdzielonej (shmat)");
        // Usuwanie pamięci współdzielonej w przypadku błędu
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("Błąd podczas usuwania pamięci współdzielonej (shmctl)");
        }
        exit(EXIT_FAILURE);
    }

    // Zerowanie pamięci
    memset(hive, 0, size);

    return hive;
}

// Funkcja do czyszczenia pamięci współdzielonej
void cleanup_shared_memory(Beehive* hive) {
    // Odłączanie pamięci współdzielonej
    if (shmdt(hive) == -1) {
        perror("Błąd podczas odłączania pamięci współdzielonej (shmdt)");
    }

    // Usuwanie pamięci współdzielonej
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("Błąd podczas usuwania pamięci współdzielonej (shmctl)");
    }
}

