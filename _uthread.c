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
 * A quueue of threads
 */
struct uthread_queue {
  uthread_t head;
  uthread_t tail;
};

/**
 * The Ready Queue
 */
struct uthread_queue ready_queue = {NULL, NULL};

/**
 * Add thread to queue.
 */
static void enqueue (struct uthread_queue* queue, uthread_t thread) {
	// TODO
	thread->next = NULL;
	if (queue->tail)
	{
		queue->tail->next = thread;
	}
	
	queue->tail = thread;
  
	if (queue->head==0)
	{
		queue->head = queue->tail;
	}
}

/**
 * Remove top thread on queue and return it.
 */
static uthread_t dequeue (struct uthread_queue* queue) {
	// TODO

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
	else
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
  	uthread_t new_thread = (uthread_t )malloc(sizeof(struct uthread_tcb));
	new_thread->arg = arg;
	new_thread->start_routine = start_routine;
	new_thread->stack =  malloc(STACK_SIZE);
	//printf("New thread initialized \n");
	//fflush(stdout);
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
	uthread_t New;
	New = uthread_new(dummy, 0);
	New->state = RUNNING;
	self = New;
	//printf("Init started = %p\n", self);
	fflush(stdout);
}

/**
 * Close the uthread system and free the current thread's thread control block.
 */
void uthread_close() {
  // TODO
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
	*thread_p = uthread_new(start_routine, arg);
	(*thread_p)->state = NASCENT;
	enqueue(&ready_queue, (*thread_p));
	//printf("Thread Created = %p \n", *thread_p);
	fflush(stdout);
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

static void uthread_start (uthread_t thread) {
  SWITCH_TO_STACK (thread);
  /*// TODO
  // call the routine with the argument and safe the result
  // TODO some more - first forget about join and detach
  // free thread assuming no join or detach was called
  // dequeue a thread
  // become the thread
  //
  // if the function calls you, we only return here once the thread finishes
  
  // if there is a joiner that is not NULL
  // unblock the joiner 
  // set state to ready
  // enqueue the joiner
  // if joiner is NULL
  // set thread state to finished   
  // else if detached
  // free */
	thread->start_routine(thread->arg);
	void **result = thread->result;

	//free(thread);
	
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
		free(thread);
	}
	uthread_t DQ = dequeue(&ready_queue);
	uthread_become(DQ);
	
	
}

/**
 * Become the specified thread.
 */
static void uthread_become (uthread_t thread) {
 /*// TODO
  // change into the stack of the other thread
  // longjump to the other state
  // if the thread is Nascent we can not longjump because it hasn't executed yet
  // therefore start the thread in uthread_start
  // change state to running */
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
	int r = setjmp(self->env);
	if (r == 0)
	{
		//printf("\n Thread = %p is in switch and ready to become \n", thread);
		//fflush(stdout);
		uthread_become(thread);
	}
}

/**
 * Yield to thread on top of ready queue, if there is one.
 */
void uthread_yield() {
	// TODO
	// safe the state of running thread
	// enqueue current thread
	// state moves from running to ready
	// dequeue new thread
	// this new thread changes state from ready to running
	// then switch to it, jump to new thread
	//
	// if there is no current thread in the queue
	// yield becomes a noop 
	// the current thread keeps running
	
	//int state = setjmp(self->env);
	
	enqueue(&ready_queue, self);
	self->state  = READY;
	uthread_t new = dequeue(&ready_queue);
	
	//if(state ==0)
	//{
		
		//if(new)
		//{
			//printf("%p is going to switch \n", new);
			//fflush(stdout);
			uthread_switch(new);
		//}
		
		//if(ready_queue.head == NULL)
		//	return;
	//}
}

/**
 * Join with specified thread.  When thread completes, copy its
 * return value into *result_p if result_p is not NULL.
 */
int uthread_join (uthread_t thread, void** result_p) {
/*// TODO
  // eventually we need to switch to another thread
  // dequeue a thread
  // switch to that thread
  // 
  // 2 cases:
  // if thread is finished
  // return result
  // and free the thread that was joined
  // if thread is not finished
  // set it to block
  // save the current thread as a joiner
  // until that thread calls unblock on us
  // dequeue the thread and switch to it
  */ 

	if(thread->state == FINISHED)
	{
		//printf("Thread %p is in Join Now and Is Finished \n", thread);
		//fflush(stdout);
		if(result_p)
		{ 
			*result_p = thread->result;
			free(thread);
			return 0;
		}
	}
	
	while(thread->state != FINISHED)
	{
		thread->joiner = self;
        block(self);
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
	thread->state = READY;
	enqueue(&ready_queue, thread);
}

/**
 * block a thread
 */

void block(uthread_t thread)
{
	//printf("\nThread = %p is in block \n",thread);
	//fflush(stdout);
	thread->state = BLOCKED;
	uthread_t sw = dequeue(&ready_queue);
	uthread_switch(sw);
}

/**
 * Dummy thread for Init
 */ 
void* dummy (void* args)
{
	while(1)
	uthread_yield();
}
