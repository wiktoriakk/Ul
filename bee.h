#ifndef BEE_PROCESS_H
#define BEE_PROCESS_H

#include <sys/types.h>
#include "beehive.h"

//deklaracje funkcji pszczoły robotnicy
void bee_process(Bee *bee); //proces pszczoły robotnicy
pid_t start_bee_process(Bee *bee); //uruchomienie procesu pszczoły robotnicy

#endif
