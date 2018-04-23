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

#include "playermsg.h"
#include "rpgstats.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define WATCHED_FILE "skeleton"
#define NUM_POLL_FDS 1
#define POLL_BLOCK -1

#define INOFITY_BUF_SIZE 4096
#define BUF_SIZE 4096

#define SAMPLE_FILE_CONTENTS "SKELETON\nJust a rackety pile of bones up to no good!\nHP: %d\n"
#define SAMPLE_RESPONSE "You hit for massive damage!\n"

static int s_hp = 10;

enum operation {
	OP_NONE,
	OP_ERROR,
	OP_READ,
	OP_WRITE
};

static void unlink_files(void)
{
	unlink(WATCHED_FILE);
}

static int generate_output(char* out, size_t out_size)
{
	return snprintf(out, out_size, SAMPLE_FILE_CONTENTS, s_hp);
}

static int read_file_then_replace(char const* filename, char *outbuf,
	size_t outbuf_size)
{
	int fd = -1;
	int readlen = 0;
	char replace_contents[BUF_SIZE] = {'\0'};
	int replace_size = 0;

	fd = open(filename, O_RDWR | O_CREAT, 0666);
	if (fd == -1) {
		perror("open failed");
		return -1;
	}

	readlen = read(fd, outbuf, outbuf_size);
	if (readlen < 0) {
		perror("read failed");
		close(fd);
		return -1;
	}
	outbuf[MIN(readlen, outbuf_size)] = '\0';

	if (strncmp("attack", outbuf, 6) == 0) {
		--s_hp;
		print_message_to_player(SAMPLE_RESPONSE);
		rpgstats_hurt_player(1);
	}

	if (ftruncate(fd, 0) == -1) {
		perror("ftruncate failed");
		close(fd);
		return -1;
	}

	if (lseek(fd, 0, SEEK_SET) != 0) {
		perror("lseek failed");
		close(fd);
		return -1;	
	}

	replace_size = generate_output(replace_contents, BUF_SIZE);

	if (write(fd, replace_contents, replace_size) != replace_size) {
		perror("write failed");
		close(fd);
		return -1;
	}

	close(fd);
	return readlen;
}

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
		readlen = read(fd, buf, INOFITY_BUF_SIZE);

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
	char file_contents[BUF_SIZE] = {'\0'};
	int readlen = -1;

	atexit(unlink_files);

	readlen = read_file_then_replace(WATCHED_FILE, file_contents, BUF_SIZE);
	if (readlen == -1) {
		fprintf(stderr, "Couldn't open: " WATCHED_FILE "\n");
		return EXIT_FAILURE;
	}

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
				// do nothing
				break;
			case OP_WRITE:
				inotify_rm_watch(fd, wd);

				readlen = read_file_then_replace(WATCHED_FILE, file_contents,
					BUF_SIZE);
				if (readlen == -1) {
					fprintf(stderr, "Couldn't open: " WATCHED_FILE "\n");
					return EXIT_FAILURE;
				}

				wd = inotify_add_watch(fd, WATCHED_FILE, IN_CLOSE);
				if (wd == -1) {
					perror("inotify_add_watch failed");
					close(fd);
					return EXIT_FAILURE;	
				}
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
