#define _GNU_SOURCE
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

double now()
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000ll + tv.tv_usec;
}

int main()
{
        int i= 0;
        int check_average[20];
        struct timeval tv;
        while(i< 20)
        {
                int fd = open("./test" , O_DIRECT|O_SYNC);
                char buffer[4];
                double this_time = now();
                int read_bytes = read(fd, &buffer, 4);
        //      printf("Buffer is %s \n", buffer);
                double cur_time = now();
                check_average[i] = cur_time - this_time;
                printf("Time with cache is %f \n", cur_time - this_time);
                i++; 
        }
	float sum = 0;
	for(i=0; i<20; i++)
		sum = sum + check_average[i];
	printf("Average latency with cache is %f \n", sum / 20);
	return 0;
}
