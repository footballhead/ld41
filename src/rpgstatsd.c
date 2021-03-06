/**
 * \file rpgstatsd.c
 * \brief rpgstatsd is a daemon that manages player stats like health.
 * \details Communication is done over a FIFO.
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

#define PERM_ONLY_USER_RW 0600
#define PERM_ALL_R_USER_W 0644

#define FIFO_NAME "rpgstatsd_controller"

#define BUFFER_SIZE 4096
#define TINY_BUFFER_SIZE 32

#define FILE_HP "hp"
static int s_hp = 10;

#define FILE_BEAR "teddy_bear"
static bool s_has_bear = false;

static void cleanup_files(void)
{
	unlink(FIFO_NAME);
	unlink(FILE_HP);
	unlink(FILE_BEAR);
}

static int file_write_stat(char const* filename, int stat)
{
	int fd = -1;
	char buffer[TINY_BUFFER_SIZE] = {'\0'};
	int statlen = 0;

	fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, PERM_ALL_R_USER_W);
	if (fd == -1) {
		perror("open stat file failed");
		return -1;
	}

	statlen = snprintf(buffer, TINY_BUFFER_SIZE, "%d\n", stat);

	if (write(fd, buffer, statlen) != statlen) {
		perror("write stat file failed");
		return -1;
	}

	close(fd);
	return 0;
}

static int change_hp(int amount)
{
	s_hp = amount;
	if (file_write_stat(FILE_HP, s_hp) == -1) {
		fprintf(stderr, "Failed to update stat: " FILE_HP);
		return -1;
	}

	return 0;
}

static int give_bear(void)
{
	s_has_bear = true;
	if (file_write_stat(FILE_HP, s_has_bear) == -1) {
		fprintf(stderr, "Failed to update stat: " FILE_BEAR);
		return -1;
	}

	return 0;
}

int main(int argc, char** argv)
{
	int fd = -1;
	int readlen = -1;
	char buffer[BUFFER_SIZE] = {'\0'};

	atexit(cleanup_files);

	if (file_write_stat(FILE_HP, s_hp) == -1) {
		fprintf(stderr, "Failed to initialize stat: " FILE_HP);
		return EXIT_FAILURE;
	}

	if (mkfifo(FIFO_NAME, PERM_ONLY_USER_RW) == -1) {
		perror("mkfifo " FIFO_NAME " failed");
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

		if (strncmp("hurt", buffer, 4) == 0) {
			print_message_to_player("Ouch! That hurt!\n");
			if (change_hp(s_hp - 1) == -1) {
				close(fd);
				return EXIT_FAILURE;
			}

			if (s_hp <= 0) {
				print_message_to_player("\n\nOh no! You have died!\n\n");
				break;
			}
		} else if (strncmp("bear", buffer, 4) == 0) {
			print_message_to_player("You got a teddy bear.\n");
			if (give_bear() == -1) {
				close(fd);
				return EXIT_FAILURE;
			}
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
