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

#define READ_SIZE 512


//
// TODO try poll http://cgi.di.uoa.gr/~ad/k22/named-pipes.pdf
//


static bool echo_input()
{
	int fd = -1;
	char buf[READ_SIZE] = {'\0'};
	
	fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
	if (fd == -1) {
		perror("open failed");
		return false;
	}

	if (read(fd, buf, READ_SIZE) < 0) {
		perror("read failed");
		close(fd);
		return false;
	}

	// Make sure it is a proper string (for now)
	buf[READ_SIZE-1] = '\0';
	printf("Client msg: %s\n", buf);

	close(fd);
	return true;
}

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
		// Try a non-blocking read.
		if (!echo_input()) {
			// do nothing... oops
		}

		// Try a non-blocking write.
		fd = open(FIFO_NAME, O_WRONLY | O_NONBLOCK);
		if (fd != -1) {
			if (write(fd, buf, buflen) < buflen) {
				perror("write failed");
				break;
			}

			close(fd);
			fd = -1;
		}

		// It should be possible for any combination of success/failure to
		// happen for read/write attemps and have the loop keep going strong.

		// I'm not sure if it's the nature of cat or the tight loop, but,
		// without this sleep(), cat tends to print out the message multiple
		// times before stopping. Since we're operating on human time, this
		// delay is probably acceptable.
		sleep(1);
	}

	if (fd != -1) {
		close(fd);
		fd = -1;
	}

	return EXIT_SUCCESS;
}
