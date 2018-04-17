# OS project 3: Concurrent programming 
## Usage 
To build the project:  
```
$make
```  
  
First we need to run the coordinator process:  
```
$./coordinator
```  
  
To run a cashier (maximum cashier number can be changed in shm.h):  
```
$./cashier -s [service time]
```  
  
To run a customer (maximum queue length can be changed in shm.h):  
```
$./customer -i [id of item to order] -e [eat time]
```  
  
To run the server:  
```
$./server
```  
  
Finally we have to run the cleaner process:  
```
$./cleaner
```  
