all: coordinator cleaner cashier customer

coordinator: coordinator.c shm.h
	gcc coordinator.c -o coordinator -lpthread

cleaner: cleaner.c shm.h
	gcc cleaner.c -o cleaner

cashier: cashier.c shm.h
	gcc cashier.c -o cashier -lpthread

customer: customer.c shm.h
	gcc customer.c -o customer -lpthread