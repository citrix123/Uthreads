#ifndef __uthread_h__
#define __uthread_h__

struct uthread_tcb;
typedef struct uthread_tcb* uthread_t;

struct waiter_queue;
typedef struct waiter_queue* queue_t;

typedef int s_lock_t;

void      uthread_init   ();
void      uthread_close  ();
int       uthread_create (uthread_t* thread, void* (*start_routine)(void*), void* arg);
int       uthread_join   (uthread_t thread, void** value_ptr);
int       uthread_detach (uthread_t thread);
void      uthread_yield  ();
uthread_t uthread_self   ();
void unblock(uthread_t thread);
void block(uthread_t thread);
void* dummy(void *args);


struct sem;
typedef struct sem* sem_t;
void sem_create(sem_t *sem_p, int initial_value);
void sem_wait (sem_t sem);
void sem_signal (sem_t sem);
void sem_destroy (sem_t sem);
void lock (int* lock_p);
void unlock (int* lock_p);

struct uthread_queue;
typedef struct uthread_queue* uthread_queue_t;
void uthread_queue_create (uthread_queue_t* queue_p);
void uthread_queue_destroy (uthread_queue_t);
void uthread_block (uthread_queue_t queue);
void uthread_unblock (uthread_queue_t queue);

int sync_val_compare_and_swap (int *value, int expected, int newvalue);


void uthread_destroy(uthread_t thread);
#endif /* __uthread_h__ */
