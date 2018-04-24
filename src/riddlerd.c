/**
 * \file riddlerd.c
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "playermsg.h"

#define PERM_RW_RW_RW 0666
#define PERM_RWX_RX_RX 0755
#define PERM_RWX_0_0 0700

#define FIFO_NAME "answer_box"
#define GUARD_DIR_NAME "maize_forest"

#define BUFFER_SIZE 4096
#define TINY_BUFFER_SIZE 32

#define MSG_FAIL "'I'm afraid that's wrong! It's okay, I have all day and I love riddles!'\n"
#define MSG_SUCCESS "'Correct! You are free to pass into maize_forest!' The riddler unlocks the gate, stands aside, and gestures that you go through.\n"

static void cleanup_files(void)
{
	unlink(FIFO_NAME);
	chmod(GUARD_DIR_NAME, PERM_RWX_RX_RX);
}

int main(int argc, char** argv)
{
	int fd = -1;
	int readlen = -1;
	char buffer[BUFFER_SIZE] = {'\0'};

	atexit(cleanup_files);

	if (chmod(GUARD_DIR_NAME, PERM_RWX_0_0) == -1) {
		perror("chmod " GUARD_DIR_NAME " failed");
		return EXIT_FAILURE;
	}

	if (mkfifo(FIFO_NAME, PERM_RW_RW_RW) == -1) {
		perror("mkfifo " FIFO_NAME " failed");
		return EXIT_FAILURE;
	}

	if (chmod(FIFO_NAME, PERM_RW_RW_RW) == -1) {
		perror("chmod " FIFO_NAME " failed");
		return EXIT_FAILURE;
	}

	fd = open(FIFO_NAME, O_RDONLY);
	if (fd == -1) {
		perror("open " FIFO_NAME " failed");
		return EXIT_FAILURE;
	}

	while (true) {
		readlen = read(fd, buffer, BUFFER_SIZE);
		if (readlen == -1) {
			perror("read " FIFO_NAME " failed");
			close(fd);
			fd = -1;
			return EXIT_FAILURE;
		}

		if (readlen == 0) {
			continue;
		}

		if (strncmp("footstep", buffer, 8) == 0) {
			print_message_to_player(MSG_SUCCESS);
			chmod(GUARD_DIR_NAME, PERM_RWX_RX_RX);
			break;
		} else {
			print_message_to_player(MSG_FAIL);
		}
	}

	// We get here when the player health drops to 0. The shutdown of this
	// process triggers root, via deathwatch, to terminate the container.

	if (fd != -1) {
		close(fd);
		fd = -1;
	}

	return EXIT_SUCCESS;
}
