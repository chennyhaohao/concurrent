#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "./shm.h"

int main() {
	key_t mem_key;
	int shm_id;
	struct shmdata *shm_ptr;
	FILE * db_fp, * ipc_fp;

	if ( (db_fp = fopen(DB_FNAME, "w")) < 0 ) { //Create empty db file
    	perror("fopen");
    	return -1;
    }

    fclose(db_fp);

    if ( (ipc_fp = fopen(IPC_FNAME, "w")) < 0 ) { //Create empty ipc file
    	perror("fopen");
    	return -1;
    }

    fclose(ipc_fp);

	if ( (mem_key = ftok(IPC_FNAME, projectID)) == -1 ) {
		perror("ftok");
		return -1;
	}

	//Create shared memory

	if ( (shm_id = shmget(mem_key, sizeof(struct shmdata), IPC_CREAT|0666)) < 0) { //get shm id
		perror("shmget");
		return -1;
	}

	printf("shmid created: %d\n", shm_id);

	if ( (long)(shm_ptr = (struct shmdata *) shmat(shm_id, NULL, 0)) == -1 ) { //attach to shm
		perror("shmat");
		return -1;
	} 

	printf("shm attached\n");

	//Initialize shared memory data structures, including semaphores

	shm_ptr->waiting = 0;
	shm_ptr->db_i = 0;
	shm_ptr->next_id = 0;
	shm_ptr->cashier_count = 0;

	sem_init(&(shm_ptr->mutex), 1, 1);
	sem_init(&(shm_ptr->db_mutex), 1, 1);
	sem_init(&(shm_ptr->cashier_available), 1, 0); //No cashier at first

	sem_init(&(shm_ptr->server_mutex), 1, 1);
	sem_init(&(shm_ptr->server_customer), 1, 0);
	sem_init(&(shm_ptr->server_available), 1, 0);
	sem_init(&(shm_ptr->server_service), 1, 0);
	sem_init(&(shm_ptr->id_updated), 1, 0);

	int i;
	for (i=0; i<maxCashier; i++) {
		shm_ptr->cashiers[i].busy = 1;
		sem_init(&(shm_ptr->cashiers[i].customer_ready), 1, 0); //No customer at first
		sem_init(&(shm_ptr->cashiers[i].service_done), 1, 0);
		sem_init(&(shm_ptr->cashiers[i].receipt_collected), 1, 0);
	}

	printf("shm initialized\n");

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}

	printf("shm detached\n");


	return 0;
}
