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

int main(int argc, char **argv) {
	char opt;
	int service_time, index, db_index, argNum = 0;
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

	if ( (int)(shm_ptr = (struct shmdata *) shmat(shm_id, NULL, 0)) == -1 ) {
		perror("shmat");
		return -1;
	} 

	printf("shm attached\n");

	sem_wait(&(shm_ptr->customer)); //Wait till there are customers

	sem_wait(&(shm_ptr->mutex)); //Mutex lock
	index = shm_ptr->head_i;
	shm_ptr->head_i = (shm_ptr->head_i+1)%maxPeople; //Move queue head forward
	sem_post(&(shm_ptr->mutex)); //Release mutex

	sleep(service_time);

	printf("Picked up order from client %d for item %d ", shm_ptr->orders[index].client_id, 
		shm_ptr->orders[index].item_id);
	struct menu_item item = getItem(menu, shm_ptr->orders[index].item_id);
	printf("Item price: %f min_t: %d max_t: %d\n", item.price, item.min_t, item.max_t);

	sem_wait(&(shm_ptr->db_mutex));
	db_index = shm_ptr->db_i;
	shm_ptr->db_i++;
	sem_post(&(shm_ptr->db_mutex));

	fseek(db_fp, db_index*sizeof(struct order), SEEK_SET);
	fwrite(&(shm_ptr->orders[index]), sizeof(struct order), 1, db_fp);

	sem_post(&(shm_ptr->queue[index])); //Wake up customer

	if ( (shmdt(shm_ptr)) == -1 ) {
		perror("shmdt");
		return -1;
	}

	printf("shm detached\n");

	fclose(db_fp);

	return 0;
}