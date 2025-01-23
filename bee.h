#ifndef BEE_PROCESS_H
#define BEE_PROCESS_H

#include <sys/types.h>
#include "beehive.h"

// Deklaracje funkcji
void bee_process(Bee *bee);
pid_t start_bee_process(Bee *bee);

#endif
