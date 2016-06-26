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
	/**
	* Algorinthm for finding dis latency
	* 1. open file
	* 2. now = gettimeofday()
	* 3. write()
	* 4. this = gettimeofday()
	* 5. this - now.
	* 6. print(this - now);
	**/

	struct timeval tv;
	int fd = open("test_thread" , O_RDWR);
        char buffer[4];
	double this_time = now();
	int read_bytes = read(fd, &buffer, 4);
//	printf("Buffer is %s \n", buffer);
	double cur_time = now();

	printf("Time without cache is %f \n", cur_time - this_time);

	return 0;
}
