#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <aio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
const int SIZE_TO_READ = 100;

double now()
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000ll + tv.tv_usec;
}

int main()
{
        int file = open("test_thread", O_RDONLY, 0);
		double Current_time, this_time;
        if (file == -1)
        {
                perror("Error in Opening File:");
                return 1;
        }

        char* buffer = malloc(sizeof(char) * SIZE_TO_READ);

        struct aiocb cb;

        memset(&cb, 0, sizeof(struct aiocb));
        cb.aio_nbytes = SIZE_TO_READ;
        cb.aio_fildes = file;
        cb.aio_offset = 0;
        cb.aio_buf = buffer;
	
	Current_time = now();
        // read!
        if (aio_read(&cb) == -1)
        {
		perror("Error reading:");
                close(file);
        }
	printf("Request enqueued \n");

        // wait until the request has finished
        while(aio_error(&cb) == EINPROGRESS)
        {
                printf("Working \n");
        }
	this_time = now() - Current_time;
	printf("Time with aio read = %f\n", this_time); 

        int ret = aio_return(&cb);

        if (ret != -1)
		printf("Success \n");
        else
		printf("Error \n");
        // now clean up
        free(buffer);
        close(file);

        return 0;
}

