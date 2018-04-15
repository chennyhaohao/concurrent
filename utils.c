#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "shm.h"

int r_rand(int rmin, int rmax) { //Returns random number between rmin and rmax (inclusive)
    if (rmax <= rmin) return rmin;
    return rand()%(rmax - rmin + 1) + rmin;
}

struct order getOrder(FILE * fp, int client_id) {
	int nread;
	struct order o;
	fseek(fp, 0, SEEK_SET);
	while (1) {
		nread = fread(&o, sizeof(struct order), 1, fp);
		if (nread == 0) {
			break;
		} else {
			if (o.client_id == client_id) return o;
		}
	}
	struct order o1 = {0,0,0.0,0};
	return o1;
}