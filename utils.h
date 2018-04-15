#include <stdlib.h>
#include <stdio.h>
#include "shm.h"

struct order getOrder(FILE * fp, int client_id);

int r_rand(int rmin, int rmax);