#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "./shm.h"

int       shm_id;
key_t     mem_key;
int       *shm_ptr;

int main() {

	struct order o;
	int nread;
	FILE * db_fp;

	if ( (db_fp = fopen("./db.bin", "r")) < 0 ) {
    	perror("fopen");
    	return -1;
    }

    while(1) {
    	nread = fread(&o, sizeof(struct order), 1, db_fp);
    	if (nread == 0) break;
    	printf("Client id: %d, item id:%d price: %f, waiting time:%d\n", o.client_id, o.item_id,
    		o.price, o.waiting_time);
    }

    fclose(db_fp);

	if ( (mem_key = ftok("./ipc.temp", projectID)) == -1 ) {
		perror("ftok");
		return -1;
	}

	if ( (shm_id = shmget(mem_key, sizeof(struct shmdata), 0666)) < 0) { //get shm id
		perror("shmget");
		return -1;
	} 

	printf("shmid to remove: %d\n", shm_id);

	if ( shmctl(shm_id, IPC_RMID, 0) < 0 ) { //rm shm using id
		perror("shmctl");
	}

	printf("shm removed\n");

	return 0;
}