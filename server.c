#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#include "./menu.h"
#include "./utils.h"

int       shm_id;
key_t     mem_key;
struct shmdata *shm_ptr;


int main(int argc, char **argv) {
	int curr_id, db_index, serve_time;
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

	if ( (long)(shm_ptr = (struct shmdata *) shmat(shm_id, NULL, 0)) == -1 ) {
		perror("shmat");
		return -1;
	} 

	printf("shm attached\n");

	srand(time(NULL));

	do {
		sem_post(&(shm_ptr->server_available)); //Wake up 1 customer
		sem_wait(&(shm_ptr->server_customer)); //Wait till there are customers

		sem_wait(&(shm_ptr->id_updated)); //Wait till customer update curr_id

		sem_wait(&(shm_ptr->server_mutex));
		curr_id = shm_ptr->curr_id;
		sem_post(&(shm_ptr->server_mutex));

		o = getOrder(db_fp, curr_id);
		printf("Got order from client %d for item %d\n", o.client_id, o.item_id);
		m = getItem(menu, o.item_id);
		printf("Item price: %f min_t: %d max_t: %d\n", m.price, m.min_t, m.max_t);
		serve_time = r_rand(m.min_t, m.max_t);
		printf("Serve time: %d\n", serve_time);
		sleep(serve_time);

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