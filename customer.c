#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#include "./utils.h"


int       shm_id;
key_t     mem_key;
struct shmdata *shm_ptr;
pid_t pid;

int main(int argc, char **argv) {
	char opt;
	int item_id, client_id, eat_time, cashier_i, db_i, argNum = 0;
	time_t start_time, waiting_time;
	char * usage_msg = "Usage %s: -i [item_id] -e [eat_time]\n";
	struct order o;
	FILE * db_fp;

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

	if ( (long)(shm_ptr = (struct shmdata *) shmat(shm_id, NULL, 0)) == -1 ) {
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

	shm_ptr->waiting++;
	sem_post(&(shm_ptr->mutex));

	start_time = time(NULL); //Start timer

	sem_wait(&(shm_ptr->cashier_available)); //Wait for available cashier

	sem_wait(&(shm_ptr->mutex)); //Mutex lock
	int i;
	for (i=0; i<maxCashier; i++) {
		if (!shm_ptr->cashiers[i].busy) {
			cashier_i = i; //pick cashier who's not busy
			shm_ptr->cashiers[i].busy = 1; //Reserve cashier
			break;
		}
	}
	printf("Going to cashier #%d\n", cashier_i);
	shm_ptr->cashiers[cashier_i].item_id = item_id; //Place order

	sem_post(&(shm_ptr->mutex)); //Release mutex
	sem_post(&(shm_ptr->cashiers[cashier_i].customer_ready)); //Wake up cashier

	sem_wait(&(shm_ptr->cashiers[cashier_i].service_done)); //Wait for service
	client_id = shm_ptr->cashiers[cashier_i].client_id;
	printf("Customer got served by cashier. Assigned id: %d\n", client_id);
	sem_post(&(shm_ptr->cashiers[cashier_i].receipt_collected)); //Acknowledge receipt

	sem_wait(&(shm_ptr->mutex)); //Mutex lock
	shm_ptr->waiting--;
	sem_post(&(shm_ptr->mutex)); //Release mutex

	sem_post(&(shm_ptr->server_customer)); //Wake up server
	sem_wait(&(shm_ptr->server_available)); //Wait for server

	sem_wait(&(shm_ptr->server_mutex));
	shm_ptr->curr_id = client_id;
	sem_post(&(shm_ptr->server_mutex));
	sem_post(&(shm_ptr->id_updated)); //Notify server that curr_id is updated

	sem_wait(&(shm_ptr->server_service)); //Wait for dish
	printf("Client got served by server; starts eating...\n");
	sleep(eat_time);

	waiting_time = time(NULL) - start_time; //Calculate waiting time
	printf("Waiting time: %d\n", (int)waiting_time);

	if ( (db_fp = fopen("./db.bin", "r+")) < 0 ) {
    	perror("fopen");
    	return -1;
    }

    db_i = getOrder(db_fp, client_id, &o);
    o.waiting_time = (int) waiting_time;
    writeOrder(db_fp, db_i, &o);

    fclose(db_fp);

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}

	printf("shm detached\n");

	return 0;
}