#ifndef BEE_PROCESS_H
#define BEE_PROCESS_H

#include <sys/types.h>
#include "beehive.h"

//deklaracje funkcji pszczoły
void bee_process(Bee *bee);
pid_t start_bee_process(Bee *bee);

#endif
