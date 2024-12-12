#include<stdio.h>
#include<stdlib.h>

//struktura pszczoly
typedef struct {
	char type; //rodzaj pszczoly: krolowa czy robotnica
	int age; //wiek (zycie)
	int visits; //liczba wizyt w ulu
} Bee;

//struktura ula
typedef struct {
	Bee* bees; 
	int total_bees; //liczba pszczol w roju
	int max_population; //maksymalna liczba pszczol jaka moze pomiescic ul
	int max_bees_in_hive; //maksymalna liczba pszczol w ulu
} Beehive; 

int main() {
	int initial_bees = 10;
	int max_population = 50;
	int max_bees_in_hive = 25;

	Beehive* hive = malloc(sizeof(Beehive));
	hive->bees = malloc(max_population * sizeof(Bee));
	hive->total_bees = initial_bees;
	hive->max_population = max_population;
	hive->max_bees_in_hive = max_bees_in_hive;

	printf("Poczatkowa liczba pszczol: %d\n", hive->total_bees);

	free(hive->bees);
	free(hive);

	return 0;
}
