#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "shm.h"

int r_rand(int rmin, int rmax) { //Returns random number between rmin and rmax (inclusive)
	srand(time(NULL));
    if (rmax <= rmin) return rmin;
    return rand()%(rmax - rmin + 1) + rmin;
}

int getOrder(FILE * fp, int client_id, struct order * o) { //Returns db index & writes order into o
	int nread, i = 0;
	fseek(fp, 0, SEEK_SET);
	while (1) {
		nread = fread(o, sizeof(struct order), 1, fp);
		if (nread == 0) { //Not found
			break;
		} else {
			if (o->client_id == client_id) return i;
		}
		i++;
	}
	return -1;
}

void writeOrder(FILE * fp, int index, struct order * o) { //Write order into db at position index
	fseek(fp, index * sizeof(struct order), SEEK_SET);
	fwrite(o, sizeof(struct order), 1, fp);
}