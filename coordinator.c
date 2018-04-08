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


	if ( (mem_key = ftok("./ipc.temp", 666)) == -1 ) {
		perror("ftok");
		return -1;
	}

	if ( (shm_id = shmget(mem_key, sizeof(struct shmdata), IPC_CREAT|0666)) < 0) { //get shm id
		perror("shmget");
		return -1;
	}

	printf("shmid created: %d\n", shm_id);

	if ( (int)(shm_ptr = (struct shmdata *) shmat(shm_id, NULL, 0)) == -1 ) { //attach to shm
		perror("shmat");
		return -1;
	} 

	printf("shm attached\n");

	shm_ptr->waiting = 0;
	shm_ptr->head_i = 0;
	shm_ptr->tail_i = 0;
	sem_init(&(shm_ptr->mutex), 1, 1);
	sem_init(&(shm_ptr->customer), 1, 0);
	for (int i=0; i<maxPeople; i++) {
		sem_init(&(shm_ptr->queue[i]), 1, 0);
	}

	printf("shm initialized\n");

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}

	printf("shm detached\n");


	return 0;
}
