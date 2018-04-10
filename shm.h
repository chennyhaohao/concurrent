#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>

#define maxPeople 3
#define projectID 2333

struct order {
	pid_t client_id;
	int item_id;
};

struct shmdata {
	int waiting;
	int head_i;
	int tail_i;
	int db_i;
	int curr_id;

	sem_t mutex;
	sem_t db_mutex;
	sem_t customer;

	sem_t server_mutex;
	sem_t server_customer;
	sem_t server_available;
	sem_t server_service;
	sem_t id_updated;

	sem_t queue[maxPeople];
	struct order orders[maxPeople];

	
};