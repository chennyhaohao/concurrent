all:  utils coordinator cleaner cashier customer server

coordinator: coordinator.c shm.h
	gcc coordinator.c -o coordinator -lpthread

cleaner: cleaner.c shm.h
	gcc cleaner.c -o cleaner -lpthread

cashier: cashier.c shm.h menu.h utils.c
	gcc cashier.c utils.o -o cashier -lpthread

customer: customer.c shm.h utils.c
	gcc customer.c utils.o -o customer -lpthread

server: server.c shm.h menu.h utils.c
	gcc server.c utils.o -o server -lpthread

utils: utils.c shm.h
	gcc utils.c -c -o utils.o
