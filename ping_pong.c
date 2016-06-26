#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"

struct param {
  int id;
  int iter;
};

void* print_and_yield (void* param_v) {
  struct param* param = param_v;
  for (int i=0; i<param->iter; i++) {
    printf ("%c", 'a' + param->id);
    fflush (stdout);
    uthread_yield();
  }
  return NULL;
}

/**
 * This should repeat the string 'abc..' where length of string is equal
 * to the number of threads and where the string is repeated for specified
 * number of iterations.
 *
 * For example:
 *   ./ping_pong 4 4
 * Should print:
 *   abcdabcdabcdabcd
 */

int main (int argc, char** argv) {
  if (argc != 3) {
    fprintf (stderr, "usage: ping_pong num-threads num-iterations\n");
    exit    (1);
  }
  int num_threads    = atoi (argv[1]);
  int num_iterations = atoi (argv[2]);
  
  uthread_init();
  
  uthread_t*    thread = malloc (num_threads * sizeof (*thread));
  struct param* param  = malloc (num_threads * sizeof (*param));
  for (int i=0; i<num_threads; i++) {
    param [i] .id   = i;
    param [i] .iter = num_iterations;
    uthread_create (&thread[i], print_and_yield, &param[i]);
  }
  for (int i=0; i<num_threads; i++)
    uthread_join (thread [i], NULL);

  for(int i=0; i<num_threads; i++)
    uthread_destroy(thread[i]);	
  free (thread);
  free (param);
  printf ("\n");
  
  uthread_close();
}
