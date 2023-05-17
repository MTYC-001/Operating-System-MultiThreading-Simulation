#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
/*
references for FIFO Queue:
https://data-flair.training/blogs/queue-in-c-cpp/ 
https://www.tutorialspoint.com/data_structures_algorithms/queue_program_in_c.htm

reference for mutex synchronization:
"Producer - Consumer Problem in Multi-Threading"
https://www.youtube.com/watch?v=l6zkaJFjUbM&t=970s
https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html

references for Structs:
https://www.w3schools.com/c/c_structs.php
*/
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;
int count = 0;
struct Customer
{
    int customerNumber;
    char serviceType;
    char* arrivalT;
};
////////////////////////////////////////////////////////////////////////////////////
//Implemented queue as a struct for easier use as well as being able to store the customer struct into queue.
struct Queue {
    struct Customer cs[100];
    int head;
    int tail;
    int size;
};
 struct Queue queue;
////////////////////////////////////////////////////////////////////////////////////
void initQueue(struct Queue* queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}
////////////////////////////////////////////////////////////////////////////////////
bool isFull()
{
	return queue.size == 100;
}
////////////////////////////////////////////////////////////////////////////////////
bool isEmpty()
{
	return queue.size == 0;
}
////////////////////////////////////////////////////////////////////////////////////
void enqueue(struct Queue* queue, struct Customer c) {
    pthread_mutex_lock(&queue_mutex);
    while (queue->size >= 100) {
        pthread_cond_wait(&queue_not_full, &queue_mutex);
    }
    if (!(isFull())) {
        if (queue->tail == 99) {
            queue->tail = -1;
        }
        queue->cs[queue->tail++] = c;
        queue->size++;
        printf("Added into queue customer with customer number = %d and service = %c and arrivalTime = %s\n", c.customerNumber, c.serviceType, c.arrivalT);
    }
    count++;
    pthread_cond_signal(&queue_not_empty);
    
    pthread_mutex_unlock(&queue_mutex);
}
////////////////////////////////////////////////////////////////////////////////////
struct Customer dequeue(struct Queue* queue) {
    pthread_mutex_lock(&queue_mutex);
    while (queue->size == 0) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    struct Customer customer = queue->cs[queue->head];
    queue->head++;
    if (queue->head == 100) {
        queue->head = 0;
    }
    queue->size--;
    count--;
    pthread_cond_signal(&queue_not_full);
    pthread_mutex_unlock(&queue_mutex);
    return customer;
}
////////////////////////////////////////////////////////////////////////////////////
void* getCustomers(void* arg) 
{
   
    int customer_number;
    char service;
    int duration;
    initQueue(&queue);
    duration = *(int*)arg;;
    bool d = false;
  while(!d)
  {

     FILE* file = fopen("c_file.txt", "r");
     FILE* f = fopen("r_log.txt", "a");
     if (file == NULL) 
     {
        printf("Error: could not open file\n");
        return 1;
     }
   
     while (fscanf(file, "%d %c", &customer_number, &service) == 2) 
     {
     
     	 time_t t = time(NULL);
   	 struct tm *tm = localtime(&t);
   	 char time_string[100];
    	 strftime(time_string, sizeof(time_string), "%H:%M:%S", tm);
         struct Customer c = {customer_number, service,time_string};
     	 fprintf(f, "Customer#%d %c %s\n", c.customerNumber, c.serviceType, c.arrivalT);
         fprintf(f, "Current time: %s\n", c.arrivalT);
         fprintf(f, "-------------------------------------------\n");
         enqueue(&queue, c);
         pthread_cond_signal(&queue_not_empty);
         sleep(duration);
        
     }
	
     fclose(file);
     fclose(f);
     d = true;
   }
   pthread_exit(NULL);
}
////////////////////////////////////////////////////////////////////////////////////
void* teller(void* arg)
{
 int teller_id = *(int*)arg;
 int wait=0;
 bool d = false;
 while(!d)
 {
    

    	FILE *f;
    	char fileName[]= "r_log.txt";
    	f = fopen(fileName,"a");
    	
        struct Customer c = dequeue(&queue);
        
        time_t t = time(NULL);
   	struct tm *tm = localtime(&t);
   	char time_string[100];
    	strftime(time_string, sizeof(time_string), "%H:%M:%S", tm);
        fprintf(f,"teller#: %d\n", teller_id);
 	fprintf(f, "Customer#%d\n", c.customerNumber);
        fprintf(f, "Arrival Time: %s\n", c.arrivalT);
        fprintf(f,"Response Time: %s\n", time_string);
        
        fprintf(f, "-------------------------------------------\n");
        
        
        printf("Teller %d Serving customer with customer number = %d and service = %c\n",teller_id, c.customerNumber, c.serviceType);
        
    sleep(2);
    wait = 0;
    
    strftime(time_string, sizeof(time_string), "%H:%M:%S", tm);
    fprintf(f,"teller#: %d\n", teller_id);
    fprintf(f, "Customer#%d\n", c.customerNumber);
    fprintf(f, "Arrival Time: %s\n", c.arrivalT);
    fprintf(f,"Completion Time: %s\n", time_string);
        
    fprintf(f, "-------------------------------------------\n");
        
        
    printf("Teller %d completed customer with customer number = %d and service = %c\n",teller_id, c.customerNumber, c.serviceType);
    sleep(2);
    if(c.customerNumber == 100)
    {
    	d = true;
    }
    fclose(f);
    
    
 }
 pthread_exit(NULL);
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	char *eq;
        int m, tc, ti, tw, td;
  	int i, thread_nums[4] = {1, 2, 3, 4};
        if (argc != 7) {
         printf("Usage: %s <eq> <m> <tc> <ti> <tw> <td>\n", argv[0]);
          return 1;
        }
  
        eq = argv[1];
        m = atoi(argv[2]);
        tc = atoi(argv[3]);
        ti = atoi(argv[4]);
        tw = atoi(argv[5]);
        td = atoi(argv[6]);
        
	pthread_t thread;
	pthread_t teller_threads[4];
	
	
	pthread_create(&thread, NULL, getCustomers, &tc);
	
    for (i = 0; i < 4; i++) {
        pthread_create(&teller_threads[i], NULL, teller, &thread_nums[i]);
    }
    pthread_join(thread, NULL);
    for (i = 0; i < 4; i++) {
        
        pthread_join(teller_threads[i], NULL);
    }
	
	
	
	
	return 0;
}
