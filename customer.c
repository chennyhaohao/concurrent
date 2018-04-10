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
pid_t pid;

int main(int argc, char **argv) {
	char opt;
	int item_id, eat_time, argNum = 0;
	char * usage_msg = "Usage %s: -i [item_id] -e [eat_time]\n";

	while ((opt = getopt(argc, argv, "i:e:")) != -1) { // Use getopt to parse commandline arguments
        switch (opt) { 
        case 'i':
            item_id = atoi(optarg);
            if (item_id < 1 || item_id > 20) {
                printf("Invalid item id.\n");
                return -1;
            }
            argNum++;
            break;

        case 'e':
        	eat_time = atoi(optarg);
            if (eat_time < 0) {
                printf("Invalid eat time.\n");
                return -1;
            }
        	argNum++;
        	break;
        

        default: 
            fprintf(stderr, usage_msg, // In case of wrong options
                    argv[0]);
            return -1;
        }
    }

    if (argNum < 2) {
    	fprintf(stderr, usage_msg, // In case of wrong options
                    argv[0]);
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

	sem_wait(&(shm_ptr->mutex)); //Mutex lock
	if (shm_ptr->waiting >= maxPeople) {
		printf("Too many people waiting, client getting out of here\n");
		sem_post(&(shm_ptr->mutex)); //Release mutex
		if ( (shmdt(shm_ptr)) == -1 ) { //Detach from shm
			perror("shmdt");
			return -1;
		}

		printf("shm detached\n");
		return -1;
	}

	pid = getpid();

	shm_ptr->waiting++;
	int index = shm_ptr->tail_i;
	shm_ptr->tail_i = (shm_ptr->tail_i+1)%maxPeople;

	shm_ptr->orders[index].client_id = pid; 
	shm_ptr->orders[index].item_id = item_id; //Place order

	sem_post(&(shm_ptr->mutex)); //Release mutex

	sem_post(&(shm_ptr->customer)); //Wake up cashier
	sem_wait(&(shm_ptr->queue[index])); //Wait for cashier to serve

	printf("Client %d got served by cashier\n", getpid());

	sem_wait(&(shm_ptr->mutex)); //Mutex lock
	shm_ptr->waiting--;
	sem_post(&(shm_ptr->mutex)); //Release mutex

	sem_post(&(shm_ptr->server_customer)); //Wake up server
	sem_wait(&(shm_ptr->server_available)); //Wait for server

	sem_wait(&(shm_ptr->server_mutex));
	shm_ptr->curr_id = pid;
	sem_post(&(shm_ptr->server_mutex));
	sem_post(&(shm_ptr->id_updated)); //Notify server that curr_id is updated

	sem_wait(&(shm_ptr->server_service)); //Wait for dish
	printf("Client got served by server\n");

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}

	printf("shm detached\n");

	return 0;
}