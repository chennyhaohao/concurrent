#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "./shm.h"

int main() {
	struct order o;
	int nread;
	FILE *fp = fopen("./db.bin", "r");
	if (fp<0) {
		perror("fopen");
		return -1;
	}

	for (int i=0; i<10; i++) {
		nread = fread(&o, sizeof(struct order), 1, fp);
		if (nread > 0) {
			printf("Order from client %d: item %d\n", o.client_id, o.item_id);
		}
	}

	return 0;
}