/**
 * \file repeater.c
 * \brief Continuously monitors a FIFO and spits out whatever comes in.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define FIFO_FILE "messages"
#define BUFFER_SIZE 4096

// I might be able to do something similar with dup2()...
int main(int argc, char** argv)
{
	int fd = -1;
	int readlen = -1;
	char buffer[BUFFER_SIZE] = {'\0'};

	fd = open(FIFO_FILE, O_RDONLY);
	if (fd == -1) {
		perror("open failed");
		return EXIT_FAILURE;
	}

	while (true) {
		readlen = read(fd, buffer, BUFFER_SIZE);
		if (readlen == -1) {
			perror("read failed");
			close(fd);
			return EXIT_FAILURE;
		}
		readlen = MIN(readlen, BUFFER_SIZE-1);
		buffer[readlen] = '\0';

		if (readlen == 0) {
			continue;
		}

		printf("%s", buffer);
	}

	printf("Out of infinite loop! This should never happen!\n");
	if (fd != -1) {
		close(fd);
	}

	return EXIT_SUCCESS;
}
