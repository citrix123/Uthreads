#define _GNU_SOURCE
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
double now()
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000ll + tv.tv_usec;
}


int main(int argc, char *argv[])
{
	int fd;
	ssize_t numRead;
	size_t length, alignment;
	off_t offset;
	char *buf;

	if(argc < 2)
	{
		printf("please specify file name \n");
		exit(EXIT_FAILURE);
	}

	length = 512;
	offset = 0;
	alignment = 4096;
	fd = open(argv[1], O_RDONLY | O_DIRECT);

	if (fd == -1)
		perror("open");
	
	buf = (char *) memalign(alignment * 2, length + alignment) + alignment;
	
	if (buf == NULL)
		perror("memalign");

	double current = now();
	if (lseek(fd, offset, SEEK_SET) == -1)
		perror("lseek");
	
	numRead = read(fd, buf, length);
	printf("time is %f \n", now() - current);
	if (numRead == -1)
		perror("read");
//	exit(EXIT_SUCCESS);
	return 0;
}

