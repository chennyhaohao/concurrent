#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "./shm.h"

#define ITEM_NUM 20

int       shm_id;
key_t     mem_key;
int       *shm_ptr;

struct item_stat {
	int item_id;
	int frequency;
	float revenue;
};

int comp(const void *elem1, const void *elem2) {
	return ((struct item_stat *)elem1)->frequency - ((struct item_stat *)elem2)->frequency;
}

int main() {

	struct order o;
	int nread;
	FILE * db_fp;
	int total_wtime = 0, c_count = 0;
	float revenue = 0.0;

	struct item_stat stats[ITEM_NUM];

	int i;
	for (i = 0; i<ITEM_NUM; i++) {
		stats[i].item_id = i+1;
		stats[i].frequency = 0;
		stats[i].revenue = 0.0;
	}

	if ( (db_fp = fopen("./db.bin", "r")) < 0 ) {
    	perror("fopen");
    	return -1;
    }

    while(1) {
    	nread = fread(&o, sizeof(struct order), 1, db_fp);
    	if (nread == 0) break;
    	printf("Client id: %d, item id:%d, money spent: %f, waiting time:%d\n", o.client_id, o.item_id,
    		o.price, o.waiting_time);
    	total_wtime += o.waiting_time;
    	revenue += o.price;
    	stats[o.item_id - 1].frequency ++;
    	stats[o.item_id - 1].revenue += o.price;
    	c_count ++;
    }

    printf("Total number of customers: %d\n", c_count);
    printf("Revenue of the day: %f\n", revenue);
    if (c_count>0) {
    	printf("Average waiting time: %d\n", total_wtime/c_count);
    }

    qsort(stats, ITEM_NUM, sizeof(struct item_stat), comp);
    printf("Most popular items:\n");
    for (i = 0; i<5; i++) {
    	struct item_stat s = stats[ITEM_NUM - i - 1];
    	printf("ID: %d, frequency: %d, revenue: %f\n", s.item_id, s.frequency, s.revenue);
    }

    fclose(db_fp);

	if ( (mem_key = ftok("./ipc.temp", projectID)) == -1 ) {
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