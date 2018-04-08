#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>

#define maxPeople 3

struct order {
	pid_t client_id;
	int item_id;
};

struct shmdata {
	int waiting;
	int head_i;
	int tail_i;

	sem_t mutex;
	sem_t customer;
	sem_t queue[maxPeople];
	struct order orders[maxPeople];
};