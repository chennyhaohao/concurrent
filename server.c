#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "./shm.h"
#include "./menu.h"

int       shm_id;
key_t     mem_key;
struct shmdata *shm_ptr;

struct order getOrder(FILE * fp, int client_id) {
	int nread;
	struct order o;
	while (1) {
		nread = fread(&o, sizeof(struct order), 1, fp);
		if (nread == 0) {
			break;
		} else {
			if (o.client_id == client_id) return o;
		}
	}
	struct order o1 = {0,0};
	return o1;
}

int main(int argc, char **argv) {
	int curr_id, db_index;
	FILE * db_fp;
	struct order o;
	struct menu_item m;

	
    if ( (db_fp = fopen("./db.bin", "r")) < 0 ) {
    	perror("fopen");
    	return -1;
    }

	if ( (mem_key = ftok("./ipc.temp", projectID)) == -1 ) {
		perror("ftok");
		return -1;
	}

	if ( (shm_id = shmget(mem_key, sizeof(struct shmdata), 0666)) < 0 ) {
		perror("shmget");
		return -1;
	} 

	printf("shmid to attach to: %d\n", shm_id);

	if ( (int)(shm_ptr = (struct shmdata *) shmat(shm_id, NULL, 0)) == -1 ) {
		perror("shmat");
		return -1;
	} 

	printf("shm attached\n");

	do {
		sem_post(&(shm_ptr->server_available)); //Wake up 1 customer
		sem_wait(&(shm_ptr->server_customer)); //Wait till there are customers

		sem_wait(&(shm_ptr->id_updated)); //Wait till customer update curr_id

		sem_wait(&(shm_ptr->server_mutex));
		curr_id = shm_ptr->curr_id;
		o = getOrder(db_fp, curr_id);
		printf("Got order from client %d for item %d\n", o.client_id, o.item_id);
		m = getItem(menu, o.item_id);
		printf("Item price: %f min_t: %d max_t: %d\n", m.price, m.min_t, m.max_t);
		sem_post(&(shm_ptr->server_mutex));

		sleep(m.min_t);

		sem_post(&(shm_ptr->server_service)); //Wake up customer
		printf("Served customer\n");
	} while(1);

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}

	printf("shm detached\n");

	fclose(db_fp);

	return 0;
}