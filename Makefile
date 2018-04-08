all: coordinator cleaner cashier customer

coordinator: coordinator.c
	gcc coordinator.c -o coordinator -lpthread

cleaner: cleaner.c
	gcc cleaner.c -o cleaner

cashier: cashier.c
	gcc cashier.c -o cashier -lpthread

customer: customer.c
	gcc customer.c -o customer -lpthread