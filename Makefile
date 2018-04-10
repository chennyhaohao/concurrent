all: coordinator cleaner cashier customer server

coordinator: coordinator.c shm.h
	gcc coordinator.c -o coordinator -lpthread

cleaner: cleaner.c shm.h
	gcc cleaner.c -o cleaner

cashier: cashier.c shm.h menu.h
	gcc cashier.c -o cashier -lpthread

customer: customer.c shm.h
	gcc customer.c -o customer -lpthread

server: server.c shm.h menu.h
	gcc server.c -o server -lpthread