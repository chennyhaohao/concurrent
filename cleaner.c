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
	if ( (mem_key = ftok("./ipc.temp", 666)) == -1 ) {
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