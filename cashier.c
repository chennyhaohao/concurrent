#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "./shm.h"

int       shm_id;
key_t     mem_key;
struct shmdata *shm_ptr;

int main() {
	if ( (mem_key = ftok("./ipc.temp", 666)) == -1 ) {
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

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}


	printf("shm detached\n");

	return 0;
}