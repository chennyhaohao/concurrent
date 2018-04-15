#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "./menu.h"
#include "./utils.h"

int       shm_id;
key_t     mem_key;
struct shmdata *shm_ptr;
struct order o;

int main(int argc, char **argv) {
	char opt;
	int service_time, cashier_i, index, db_index, argNum = 0;
	char* usage_msg = "Usage: %s -s [service time]\n";
	FILE * db_fp;

	while ((opt = getopt(argc, argv, "s:")) != -1) { // Use getopt to parse commandline arguments
        switch (opt) { 
        case 's':
            service_time = atoi(optarg);
            if (service_time < 0) {
                printf("Invalid item id.\n");
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

    if (argNum < 1) {
    	fprintf(stderr, usage_msg, // In case of wrong options
                    argv[0]);
    	return -1;
    }

    if ( (db_fp = fopen("./db.bin", "a")) < 0 ) {
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

	sem_wait(&(shm_ptr->mutex));
	if (shm_ptr->cashier_count >= maxCashier) {
		printf("Enough cashiers today, cashier go home and watch Netflix\n");
		sem_post(&(shm_ptr->mutex)); //Release mutex
		if ( (shmdt(shm_ptr)) == -1 ) { //Detach from shm
			perror("shmdt");
			return -1;
		}

		printf("shm detached\n");
		return -1;
	}

	cashier_i = shm_ptr->cashier_count;
	printf("Cashier #%d in position\n", cashier_i);
	shm_ptr->cashier_count++;
	sem_post(&(shm_ptr->mutex));

	do {
		sem_wait(&(shm_ptr->mutex));
		shm_ptr->cashiers[cashier_i].busy = 0; //No longer busy
		sem_post(&(shm_ptr->mutex));

		sem_post(&(shm_ptr->cashier_available)); //Wake up 1 customer
		sem_wait(&(shm_ptr->cashiers[cashier_i].customer_ready)); //Wait for customers to come and place order

		sem_wait(&(shm_ptr->mutex));
		o.item_id = shm_ptr->cashiers[cashier_i].item_id; //Consume item id
		o.client_id = shm_ptr->next_id;
		shm_ptr->cashiers[cashier_i].client_id = o.client_id; //Produce client id
		shm_ptr->next_id++;
		sem_post(&(shm_ptr->mutex));

		printf("Picked up order from client %d for item %d ", o.client_id, 
			o.item_id);
		struct menu_item item = getItem(menu, o.item_id);
		printf("Item price: %f min_t: %d max_t: %d\n", item.price, item.min_t, item.max_t);

		sem_wait(&(shm_ptr->db_mutex));
		db_index = shm_ptr->db_i;
		shm_ptr->db_i++;
		sem_post(&(shm_ptr->db_mutex));

		fseek(db_fp, db_index*sizeof(struct order), SEEK_SET);
		fwrite(&o, sizeof(struct order), 1, db_fp);
		fflush(db_fp); //Make sure db is updated before customer goes to server
		sleep(service_time);

		sem_post(&(shm_ptr->cashiers[cashier_i].service_done)); //Wake up customer
		sem_wait(&(shm_ptr->cashiers[cashier_i].receipt_collected)); //Wait for ACK for receipt

	} while(1);

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}

	printf("shm detached\n");

	fclose(db_fp);

	return 0;
}