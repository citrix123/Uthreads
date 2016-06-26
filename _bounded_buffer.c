#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"

#define BUFFSIZE 5

sem_t mx, items, slot;

int buffer[BUFFSIZE], head=0, tail=0;

int insert_item(int item)
{
	sem_wait(slot);
	sem_wait(mx);
	buffer[head] = item;
	head = (head + 1) % BUFFSIZE;
	sem_signal(mx);
	sem_signal(items);
	return 0;
}

int remove_item (int *item)
{
	sem_wait(items);
	sem_wait(mx);
	
	*item = buffer[tail];
	
	tail = (tail+1)%BUFFSIZE;
	sem_signal(mx);
	sem_signal(slot);
	return 0;
}


void *producer (void *args)
{
  int i =0, success;
   while ( i < 50 )
    {
       if(insert_item(i) == 0)
       {	
			printf("producer produced %d \n", i);
			for (int j = 0; j < rand() % 10; j++)
			{
				uthread_yield();
			}

	   }
	   i ++;
    }

}

void *consumer (void *args)
{
  int i =0;
   while ( i < 50 )
    {
       if(remove_item(&i) == 0)
       {
		   printf("consumer consumed %d \n", i);
		   for (int j = 0; j < rand() % 10; j++)
			{
				uthread_yield();
			}
	   }
	   //printf("Value of i in consumer  = %d \n",i);
       i++;;
    }

}



/* My defination
void *producer (void *args)
{
	sem_wait(slot);
	sem_wait(mx);
	
	buffer[head] = (int )args;
	printf("Producer = %d \n", buffer[head]);
	head = (head + 1) % BUFFSIZE;
	
	
}

void *consumer (void * args)
{
	sem_wait(item);
	sem_wait(mx);
	
	//(int *)args = buffer[tail];
	printf("Consumer =  %d \n", buffer[tail]);
	fflush(stdout);
	tail = (tail + 1) % BUFFSIZE;
	
	sem_signal(mx);
	sem_signal(slot);
}
*/

int main()
{
	uthread_init();
	
	sem_create(&mx, 1);
	sem_create(&items, 0);
	sem_create(&slot, BUFFSIZE);

	uthread_t*    thread = malloc (4 * sizeof (*thread));
	uthread_create(&thread[0],producer, NULL);
	uthread_create(&thread[1], consumer, NULL);
	
	//uthread_create(&thread[2],producer, NULL);
	//uthread_create(&thread[3], consumer, NULL);
		
	uthread_join(thread[0], NULL);
	uthread_join(thread[1], NULL);
	
	//uthread_join(thread[2], NULL);
	//uthread_join(thread[3], NULL);
	printf("Finishing line \n");
	uthread_close();
	free(thread);
	thread = NULL;

	sem_destroy(mx);
	sem_destroy(slot);
	sem_destroy(items);
	return 0;
}

