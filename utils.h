#include <stdlib.h>
#include <stdio.h>
#include "shm.h"

int getOrder(FILE * fp, int client_id, struct order * o);

int r_rand(int rmin, int rmax);

void writeOrder(FILE * fp, int index, struct order * o);