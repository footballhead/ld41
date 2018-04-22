#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIFO_NAME "test"
#define FIFO_PERMS_ALL_RW 0666

#define MESSAGE_STRING "Hello world\n"

#define READ_SIZE 512

#define POLL_WAIT_MS 500


//
// TODO try poll http://cgi.di.uoa.gr/~ad/k22/named-pipes.pdf
//


static bool echo_input()
{
	int fd = -1;
	int rc = 0;
	char buf[READ_SIZE] = {'\0'};
	struct pollfd fdarray[1] = {0};
	
	fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
	if (fd == -1) {
		perror("read-side open failed");
		return false;
	}

	fdarray[0].fd = fd;
	fdarray[0].events = POLLIN;

	rc = poll(fdarray, sizeof(fdarray), POLL_WAIT_MS);

	// Poll error, fatal
	if (rc == -1) {
		perror("poll failed");
		close(fd);
		return false;
	}

	// Poll timeout, non-fatal
	if (rc == 0) {
		close(fd);
		return true;
	}

	// Poll was fine but not ready, non-fatal
	if (fdarray[0].revents != POLLIN) {
		close(fd);
		return true;
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

static bool print_test_message_to_readers()
{
	int fd = -1;
	int rc = 0;
	char* buf = MESSAGE_STRING;
	size_t buflen = strlen(buf);
	struct pollfd fdarray[1] = {0};

	fd = open(FIFO_NAME, O_WRONLY | O_NONBLOCK);
	if (fd == -1) {
		perror("write-side open failed");
		return false;
	}

	fdarray[0].fd = fd;
	fdarray[0].events = POLLOUT;

	rc = poll(fdarray, sizeof(fdarray), POLL_WAIT_MS);

	// Poll error, fatal
	if (rc == -1) {
		perror("poll failed");
		close(fd);
		return false;
	}

	// Poll timeout, non-fatal
	if (rc == 0) {
		close(fd);
		return true;
	}

	// Poll was fine but not ready, non-fatal
	if (fdarray[0].revents != POLLOUT) {
		close(fd);
		return true;
	}

	if (write(fd, buf, buflen) < buflen) {
		perror("write failed");
		close(fd);
		return false;
	}

	close(fd);
	return true;
}

int main(int argc, char** argv)
{
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
			return EXIT_FAILURE;
		}

		// Try a non-blocking write.
		if (!print_test_message_to_readers()) {
			return EXIT_FAILURE;
		}

		// It should be possible for any combination of success/failure to
		// happen for read/write attemps and have the loop keep going strong.
	}

	return EXIT_SUCCESS;
}
