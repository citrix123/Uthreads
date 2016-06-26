#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <stddef.h>
#include "uthread.h"

/**
 * The maximum size of a uthread stack
 */
const int STACK_SIZE = 32 * 1024 * 1024;

/**
 * Thread state
 */
enum uthread_state {
  NASCENT, RUNNING, READY, BLOCKED, FINISHED
};

/**
 * Thread Control Block
 */
struct uthread_tcb {
  enum uthread_state state;
  void*             stack;
  void*             (*start_routine) (void*);
  void*             arg;
  jmp_buf           env;
  uthread_t         joiner;
  void*             result;
  uthread_t         next;
};

/**
 * waiting_queue queue
 **/
struct uthread_queue{
	uthread_t head;
	uthread_t tail;
}; 

typedef struct uthread_queue* uthread_queue_t;  

/** 
 * semaphor's structure
 **/ 
struct sem {
	int value;
	uthread_queue_t waiting_queue;
	s_lock_t lock;	
};


struct uthread_queue ready_queue = {NULL, NULL};

/**
 * Add thread to queue.
 */
static void enqueue (struct uthread_queue* queue, uthread_t thread) {
	// TODO
	// inserting new thread in given queue's tail
	thread->next = NULL;
	if (queue->tail)  //if tail have some thread
	{
		queue->tail->next = thread;
	}
	
	queue->tail = thread;
  
	if (queue->head==0)  //if head is NULL
	{
		queue->head = queue->tail;
	}
}

/**
 * Remove top thread on queue and return it.
 */
static uthread_t dequeue (struct uthread_queue* queue) {
	// TODO 
	// removing thread from head of queue 

	uthread_t thread;
	if (queue->head) 
	{    
		thread = queue->head;
		queue->head = queue->head->next;
		if (queue->head==0)
		{
			queue->tail=0;
		}
	} 
	else  //if head have no thread
	{ 

		thread=0;		
	}
	
	return thread;
}

/**
 * Allocate and initialize a new thread control block and its stack.
 */
static uthread_t uthread_new (void* (*start_routine)(void*), void* arg) {
  // TODO
  //  allocating new thread and its stack
  //  asssigning start routine and other arguments to  allocated thread.
  	uthread_t new_thread = (uthread_t )malloc(sizeof(struct uthread_tcb));
	new_thread->arg = arg;
	new_thread->start_routine = start_routine;
	new_thread->stack = malloc(STACK_SIZE);
	new_thread->joiner = 0;

	return new_thread;
}

/**
 * Pointer to thread control block of currently executing thread.
 */
static uthread_t self;

/**
 * Initialize thread system by creating a dummy uthread for running thread,
 * setting its state to RUNNING, and setting self to point to it.
 */
void uthread_init() {
  // TODO
  //  initialising dummy thread

	uthread_t New;
	New = uthread_new(dummy, 0);
	New->state = RUNNING;
	self = New;
}

/**
 * Close the uthread system and free the current thread's thread control block.
 */
void uthread_close() {
  // TODO
    //  if stack is not NULL free the stack 
    //  and free self 
    //printf("self->state = %d \n", self->state);
    //fflush(stdout);
	if(self->stack)
	{
		free(self->stack);
	}
	free(self);
		
}


/**
 * Create a new thread and add it to the ready queue.
 * Return 0 to indicate success.
 */
int uthread_create (uthread_t* thread_p, void* (*start_routine)(void*), void* arg) {
	// TODO
	// creating a new thread
	*thread_p = uthread_new(start_routine, arg);
	(*thread_p)->state = NASCENT;
	enqueue(&ready_queue, (*thread_p));
	//printf("Thread Created = %p \n", (*thread_p)->stack);
	//fflush(stdout);
	return 0;
}

/**
 * Switch current thread's stack to that of specified thread.
 */
#if __LP64__
#define SWITCH_TO_STACK(THREAD) \
  asm volatile ("movq %c2(%1), %%rsp\n" \
                "addq $%c3,    %%rsp\n" \
                "movq %%rsp,   %%rbp\n" \
                "subq $64,     %%rsp\n" \
                "subq $32,     %%rbp\n" \
                "movq %1,      %0\n" \
                : \
                /* 0 */ "=m" (THREAD) \
                : \
                /* 1 */ "r" (THREAD), \
                /* 2 */ "i" (offsetof (struct uthread_tcb, stack)), \
                /* 3 */ "i" (STACK_SIZE));
#else
#define SWITCH_TO_STACK(THREAD) \
  asm volatile ("movl %c2(%1), %%esp\n" \
                "addl $%c3,    %%esp\n" \
                "movl %%esp,   %%ebp\n" \
                "subl $64,     %%esp\n" \
                "subl $32,     %%ebp\n" \
                "movl %1,      %0\n" \
                : \
                /* 0 */ "=m" (THREAD) \
                : \
                /* 1 */ "r" (THREAD), \
                /* 2 */ "i" (offsetof (struct uthread_tcb, stack)), \
                /* 3 */ "i" (STACK_SIZE));
#endif

/**
 * Forward reference for uthread_become so that it can be
 * called before it is defined.
 */
static void uthread_become (uthread_t);

/**
 * The base procedure of a new uthread.  This procedure is called
 * once for each NASCENT thread the first time the thread is switched to.
 * It calls the start routine with specified argument and then handles
 * possible joiner when that routine requires.  This procedure does not
 * return; it ends by dequeing a thread from the ready queue and becomeing it.
 */

static void uthread_start (uthread_t thread) 
{
	//  start routine of given thread
	//  if thread is not deattach
	//  if no joiner then change state to finish,
	//  else enqueue the thread, change the state 
	//  else free the thread
	//  dequeue a new thread, become it
	
    SWITCH_TO_STACK (thread);
	thread->start_routine(thread->arg);
	void **result = thread->result;
	
	if(thread->joiner != (uthread_t) -1)
	{
		if(thread->joiner)
		{
			unblock(thread->joiner);
			thread->joiner->state = READY;
			enqueue(&ready_queue, thread->joiner);
		}
		else if(thread->joiner == NULL)
		{
			thread->state = FINISHED;
		}
	
	}
	else
	{
		if(thread->stack)
			free(thread->stack);
		free(thread);
	}
	uthread_t DQ = dequeue(&ready_queue);
	uthread_become(DQ);
	
	
}

/**
 * Become the specified thread.
 */
static void uthread_become (uthread_t thread) 
{
	//  if thread is new start it
	//  else if ready , then change the state using long jump
	if(thread->state == NASCENT)
	{
		self = thread;
		uthread_start(thread);
	}
	else if(thread->state == READY)
	{
		self = thread;
		longjmp(thread->env, 1);
	}	
}

/**
 * Save current thread's state and then become specified thread.
 */
static void uthread_switch (uthread_t thread) {
	// TODO
	//  save the state, switch to given thread
	int r = setjmp(self->env);
	if (r == 0)
	{
		uthread_become(thread);
	}
}

/**
 * Yield to thread on top of ready queue, if there is one.
 */
void uthread_yield() {
	// TODO
	//  enqueue current thread to ready queue 
	//  change state to raedy 
	//  dequeue a new thread and switch to it
	
	enqueue(&ready_queue, self);
	self->state  = READY;
	uthread_t new = dequeue(&ready_queue);
	if(new != self)
	{
		uthread_switch(new);
	}
	else
		exit(1);
}

/**
 * Join with specified thread.  When thread completes, copy its
 * return value into *result_p if result_p is not NULL.
 */
int uthread_join (uthread_t thread, void** result_p) 
{
	//  if thread is finish save result 
	//  free the thread
	//  otherwise wait till finished
	if(thread->state == FINISHED)
	{
		if(result_p)
		{ 
			*result_p = thread->result;
			if(thread->stack)
			{
				free(thread->stack);
			}
			free(thread);
			return 0;
		}
	}
	
	while(thread->state != FINISHED)
	{
		thread->joiner = self;
        block(self);
        self->state = FINISHED;
        return 0;
	}		
}

/**
 * Detach specified thread.  If the thread has finished, free it.
 * If not, flag that there will be no joiner so that uthread_start
 * will free the thread when its start procedure returns.
 */
int uthread_detach (uthread_t thread) {
  // TODO
 
	if(thread->state == FINISHED)
	{	
		if(thread->stack)
			free(thread->stack);
		free(thread);
	}		
	else
		thread = (uthread_t) -1;
  return 0;
}

/**
 * Return pointer to thread control block of currently running thread.
 */
uthread_t uthread_self() {
  // TODO
  return self;
}

/**
 * Unblock a thread
 */
 
void unblock(uthread_t thread)
{
	// change state to ready 
	//  enqueue it to ready queue
	thread->state = READY;
	enqueue(&ready_queue, thread);
}

/**
 * block a thread
 */

void block(uthread_t thread)
{
	//  change state to block
	//  dequeue a thread and switch to it
	thread->state = BLOCKED;
	uthread_t sw = dequeue(&ready_queue);
	uthread_switch(sw);
}

/**
 * Dummy thread for Init
 */ 
void* dummy (void* args)
{
	//while(1)
	uthread_yield();
}

/**
 * Queue Create
 * Creating a New queue
 **/
void uthread_queue_create (uthread_queue_t* queue_p)
{
	//  creating a new queue for semaphor 
	*queue_p = malloc(sizeof(struct uthread_queue));
	(*queue_p)->head = NULL;
	(*queue_p)->tail = NULL;
}

/**
 * Queue Destroy
 **/
void uthread_queue_destroy (uthread_queue_t queue_p)
{
	//  free the given queue
	free(queue_p);
}

/** 
 * Blocking Queue
 * uthread_block() is similar to uthread_yield except that the calling thread is placed on the specified queue instead of the ready queue 
 * and it status is set to BLOCKED instead of READY;
 **/ 
void uthread_block (uthread_queue_t queue)
{
	//  enqueue current thread 
	//  change state to blocked
	enqueue(queue, self);
	self->state = BLOCKED;
}

/**
 * Ublocking queue
 * uthread_unblock() removes the thread at the top of the specified queue, 
 * changes its status to READY, and adds it to the ready queue.
 **/
void uthread_unblock (uthread_queue_t queue)
{
	//  dequeue the thread 
	//  if thread is not NULL, change state to ready,
	//  and enqueue it to ready queue
	uthread_t thread = dequeue(queue);
	if(thread != NULL)
	{
		thread->state = READY;
		enqueue(&ready_queue, thread);  
	}

	
}


/**
 * semaphore create
 **/
void sem_create(sem_t *semaphor_t, int initial_value)
{
	// allocating semaphor, and it's queue 
	// initialising value as initial value 
	//  and lock as 0
	*semaphor_t = (sem_t) malloc(sizeof(struct sem));
	(*semaphor_t)->value = initial_value;
	(*semaphor_t)->lock = 0;
	uthread_queue_create(&(*semaphor_t)->waiting_queue);
	//printf("Creating sem = %p \n", *semaphor_t);
	//fflush(stdout);
}

/**
 * semaphore wait
 **/ 
void sem_wait (sem_t semaphor_t)
{
	//  lock semaphor 
	//  while value is 0, run dequeueed thread from ready queue,
	//  enqueue current thread to semaphor_t queue ,
	//  unlock it, switch to it, lock it .
	//  decrease the value of semaphor, and lock it
	lock(&semaphor_t->lock);
	
	while(semaphor_t->value == 0)
	{
		uthread_t To_Run = dequeue(&ready_queue);
		enqueue(semaphor_t->waiting_queue, self);
		unlock(&semaphor_t->lock);
		uthread_switch(To_Run);
		lock(&semaphor_t->lock);
	}
	semaphor_t->value --;
	unlock(&semaphor_t->lock);
}

/**
 * semaphore signal
 **/  
void sem_signal (sem_t semaphor_t)
{
	//  lock semaphor_t, increase it's value 
	//  unblock thread and unlock it
	lock(&semaphor_t->lock);
	semaphor_t->value ++;
	uthread_unblock(semaphor_t->waiting_queue);
	unlock(&semaphor_t->lock);
}	

/**
 * Destroy semaphores
 **/  
void sem_destroy (sem_t semaphor_t)
{ 
	//  free semaphor and it's queue 
	uthread_queue_destroy(semaphor_t->waiting_queue);
	free(semaphor_t);
}

int Lock_helper (int *value, int expected, int newvalue)
{
	//  helper function for lock
	int temp = *value;
	if(*value == expected)
	{
		*value = newvalue;
	}
	return temp;	
}

void lock (int* lock) 
{
	//  checking for the value as 0
	while (Lock_helper (lock, 0, 1)) {}
}

void unlock (int* lock) 
{
	*lock = 0;
}

void uthread_destroy(uthread_t thread)
{
	if(thread->stack)
		free(thread->stack);
	free(thread);
	
}
