#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>

#define maxPeople 5
#define maxCashier 3
#define projectID 2333

struct order {
	int client_id;
	int item_id;
	float price;
	int waiting_time;
};

struct cashier_ds {
	sem_t service_done;
	sem_t customer_ready;
	sem_t receipt_collected;
	int busy;
	int item_id;
	int client_id;
};


struct shmdata {
	int waiting;
	int cashier_count;
	int db_i;
	int curr_id;
	int next_id;

	sem_t mutex;
	sem_t db_mutex;
	sem_t cashier_available;

	sem_t server_mutex;
	sem_t server_customer;
	sem_t server_available;
	sem_t server_service;
	sem_t id_updated;

	struct cashier_ds cashiers[maxCashier];	
};