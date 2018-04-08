#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/times.h>
#include <semaphore.h>
#include <sys/mman.h>

#define max_wait 20

static int *waiting, *curr_q_index, *curr_s_index;

sem_t *q_mutex, *q_sem[max_wait];

int cashier_num = 3;
int client_num = 25;

int main() {
	waiting = mmap(NULL, sizeof *waiting, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0); //Num of customers waiting
	curr_q_index = mmap(NULL, sizeof *curr_q_index, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0); //Tail of queue
	curr_s_index = mmap(NULL, sizeof *curr_s_index, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0); //Head of queue
	q_mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	for (int i=0; i<max_wait; i++) {
		q_sem[i] = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	}

	*waiting = 0;
	*curr_q_index = 0;
	*curr_s_index = 0;

	sem_init(q_mutex, 1, 1);
	for (int i=0; i<max_wait; i++) {
		sem_init(q_sem[i], 1, 0);
	}

	for (int i=0; i<client_num; i++) {
		/*if(execlp('./client', './client') < 0) {
			perror("Exec client");
			return -1;
		}*/
		if (fork() == 0) {
			sem_wait(q_mutex);
			if (*waiting == max_wait) {
				printf("Restaurant full, client %d leaving\n", i);
				sem_post(q_mutex);
				return 0;
			} else {
				*waiting = *waiting + 1;
				int j = *curr_q_index;
				*curr_q_index = (*curr_q_index+1)%max_wait;
				printf("Client %d waiting on position %d\n", i, j);
				sem_post(q_mutex); //Release mutex
				sem_wait(q_sem[j]); //Wait for cashier
				printf("Client %d at position %d got served\n", i, j);
				return 0;
			}
		}
	}

	for (int i=0; i<cashier_num; i++) {
		if (fork() == 0) { //Child
			/*if(execlp('./cashier', './cashier') < 0) {
				perror("Exec cashier");
				return -1;
			}*/
			while (1) {
				sem_wait(q_mutex);
				if (*waiting == 0) {
					printf("No one waiting, cashier %d sleeps...\n", i);
					sem_post(q_mutex); //Release mutex
					sleep(6);
				} else {
					int j = *curr_s_index;
					*curr_s_index = (*curr_s_index+1)%max_wait;
					printf("Cashier %d serving client at %d\n", i, j);
					sem_post(q_sem[j]); //Call up client
					*waiting = *waiting - 1;
					sem_post(q_mutex); //Release mutex
				}
			}
		}
	}

	for (int i=0; i<client_num; i++) {
		/*if(execlp('./client', './client') < 0) {
			perror("Exec client");
			return -1;
		}*/
		if (fork() == 0) {
			sem_wait(q_mutex);
			if (*waiting == max_wait) {
				printf("Restaurant full, client %d leaving\n", i);
				sem_post(q_mutex);
				return 0;
			} else {
				*waiting = *waiting + 1;
				int j = *curr_q_index;
				*curr_q_index = (*curr_q_index+1)%max_wait;
				printf("Client %d waiting on position %d\n", i, j);
				sem_post(q_mutex); //Release mutex
				sem_wait(q_sem[j]); //Wait for cashier
				printf("Client %d at position %d got served\n", i, j);
				return 0;
			}
		}
	}
	

	return 0;
}