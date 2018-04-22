#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
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

	// Support repeatable client reads (cat). For client to receive EOF and
	// stop reading, the pipe must be closed. Thus, this loop must be
	// responsible for reopening the file.
	while (true) {
		fd = open(FIFO_NAME, O_WRONLY);
		if (fd == -1) {
			perror("open failed");
			return EXIT_FAILURE;
		}

		if (write(fd, buf, buflen) < buflen) {
			perror("write failed");
			break;
		}

		close(fd);
		fd = -1;

		// I'm not sure if it's the nature of cat or the tight loop, but,
		// without this sleep(), cat tends to print out the message multiple
		// times before stopping. Since we're operating on human time, this
		// delay is probably acceptable.
		sleep(1);
	}

	if (fd != -1) {
		close(fd);
	}

	return EXIT_SUCCESS;
}
