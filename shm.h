#include <semaphore.h>
#define maxPeople 20


struct shmdata {
	int waiting;
	int head_i;
	int tail_i;

	sem_t mutex;
	sem_t customer;
	sem_t queue[maxPeople];
	int orders[maxPeople];
};