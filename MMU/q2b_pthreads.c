#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#define number_thread 10

double now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000ll + tv.tv_usec;
}

FILE * fd;
char *file = "test_thread";

pthread_mutex_t input_queue;

void do_work(unsigned long buf)
{
    //printf("I am getting on %u\n",buf);
}

void * work(void * data)
{
    unsigned long line;
    char buf[512];

    while ( !feof(fd) )
      {
        pthread_mutex_lock(&input_queue);
        fgets((char *)&buf, sizeof(buf), fd);
        if (buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';
        line = (unsigned long)buf;
        pthread_mutex_unlock(&input_queue);
        do_work( line );
      }

    return NULL;
}

int main()
{
    pthread_mutex_init(&input_queue, NULL);
    if( ( fd = fopen( file, "a+" ) ) == NULL ) {
        fprintf( stderr, "Can't open input file\n");
        exit(0);
    }
    pthread_t thread_id[10];

    int i=0;
    for( i = 0 ; i < number_thread; i++)
      { 
        if( pthread_create( &thread_id[i], NULL, &work, NULL) != 0 )
          {
            i--;
            fprintf(stderr,  "\nError in creating thread\n");
          }
      }
	double current = now();	
    for( i = 0 ; i < number_thread; i++)
        if( pthread_join( thread_id[i], NULL) != 0 )
          {
            fprintf(stderr,  "\nError in joining thread\n" );
          }
    printf("Time consumed by 10 threads in reading is %f \n", now() - current);
    fclose(fd);
}
