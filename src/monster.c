#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIFO_NAME "test"

#define MESSAGE_STRING "Hello world\n"

int main(int argc, char** argv)
{
	int fd = -1;
	char* buf = MESSAGE_STRING;
	size_t buflen = strlen(buf);

	if (mkfifo(FIFO_NAME, 0666) == -1) {
		perror("mkfifo failed");
		return EXIT_FAILURE;
	}

	fd = open(FIFO_NAME, O_WRONLY);
	if (fd == -1) {
		perror("open failed");
		return EXIT_FAILURE;
	}

	if (write(fd, buf, buflen) < buflen) {
		perror("write failed");
	}

	close(fd);

	return EXIT_SUCCESS;
}
