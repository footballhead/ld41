#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WATCHED_FILE "test"
#define NUM_POLL_FDS 1
#define POLL_BLOCK -1

#define INOFITY_BUF_SIZE 4096

enum operation {
	OP_NONE,
	OP_ERROR,
	OP_READ,
	OP_WRITE
};

static int handle_inotify_events(int fd, int wd)
{
	// From http://man7.org/linux/man-pages/man7/inotify.7.html:

	/* Some systems cannot read integer variables if they are not
	properly aligned. On other systems, incorrect alignment may
	decrease performance. Hence, the buffer used for reading from
	the inotify file descriptor should have the same alignment as
	struct inotify_event. */

	char buf[INOFITY_BUF_SIZE]
		__attribute__ ((aligned(__alignof__(struct inotify_event))));

	ssize_t readlen = 0;
	char* iter = buf;
	const struct inotify_event* event;
	int myerrno = 0;

	// This loop allows us to keep reading events until the fd is dry. Once the
	// fd is empty then we exit out so we can go back to poll()ing.
	do {
		// Read into a generic character buffer first.
		printf("DEBUG: read\n");
		readlen = read(fd, buf, INOFITY_BUF_SIZE);
		printf("DEBUG: readlen=%zd\n", readlen);

		// Disambiguate between fatal and non-fatal errors.
		if (readlen < 0) {
			myerrno = errno;

			// We're non-blocking so this is expected
			if (myerrno == EAGAIN) {
				break;
			}

			perror("read failed");
			return OP_ERROR;
		}

		// This loop interprets the read() values, one inotify_event at a time
		while (iter < buf + readlen) {
			event = (const struct inotify_event*)iter;
			iter += sizeof(struct inotify_event) + event->len;

			if (event->wd != wd) {
				continue;
			}

			// Look at the first valid event and ignore the rest for now.
			if (event->mask & IN_CLOSE_NOWRITE) {
				return OP_READ;
			} else if (event->mask & IN_CLOSE_WRITE) {
				return OP_WRITE;
			}
		}
	} while (readlen > 0);

	return OP_NONE;
}

int main(int argc, char** argv)
{
	int fd = -1;
	int wd = -1;
	int pollnum = -1;
	int oper = OP_NONE;
	struct pollfd pollfds[NUM_POLL_FDS];

	// Notice the use of inotify_init1. We need to provide IN_NONBLOCK so that
	// read()ing the inotify_events out of the watched descriptor doesn't block
	// and cause us to hang.
	fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
		perror("inotify_init failed");
		return EXIT_FAILURE;
	}

	pollfds[0].fd = fd;
	pollfds[0].events = POLLIN;

	wd = inotify_add_watch(fd, WATCHED_FILE, IN_CLOSE);
	if (wd == -1) {
		perror("inotify_add_watch failed");
		close(fd);
		return EXIT_FAILURE;	
	}

	// The main loop: forever poll() then read() the inotify_events for the test
	// file.
	while (true) {
		printf("DEBUG: poll\n");
		pollnum = poll(pollfds, NUM_POLL_FDS, POLL_BLOCK);
		if (pollnum == -1) {
			perror("poll failed");
			close(fd);
			fd = -1;
			return EXIT_FAILURE;
		}

		// 0 is for poll() timeout and requires us to re-poll(). However, we
		// block on poll() so this should never happen.
		if (pollnum == 0) {
			continue;
		}

		if (pollfds[0].revents & POLLIN) {
			oper = handle_inotify_events(fd, wd);

			switch (oper) {
			case OP_ERROR:
				close(fd);
				fd = -1;
				return EXIT_FAILURE;
			case OP_READ:
				printf("file read from!\n");
				break;
			case OP_WRITE:
				printf("file written to!\n");
				break;
			case OP_NONE:
				// do nothing
				break;
			}
		}
	}

	// Theoretically shouldn't get here.
	printf("Exited endless loop somehow!\n");

	if (fd != -1) {
		// Closing the inotify fd will release all watches
		close(fd);
	}

	return EXIT_SUCCESS;
}
