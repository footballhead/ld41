#include "rpgstats.h"

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#define FIFO_NAME "/world/stats/rpgstatsd_controller"

int rpgstats_hurt_player(int amount)
{
	int fd = -1;
	char const* message = "hurt";
	int len = strlen(message);

	(void)amount;

	fd = open(FIFO_NAME, O_WRONLY);
	if (fd == -1) {
		perror("librpgstats: open failed");
		return -1;
	}

	if (write(fd, message, len) != len) {
		perror("librpgstats: write failed");
		close(fd);
		return -1;	
	}

	close(fd);

	return 0;
}

int rpgstats_give_bear(void)
{
	int fd = -1;
	char const* message = "bear";
	int len = strlen(message);

	fd = open(FIFO_NAME, O_WRONLY);
	if (fd == -1) {
		perror("librpgstats: open failed");
		return -1;
	}

	if (write(fd, message, len) != len) {
		perror("librpgstats: write failed");
		close(fd);
		return -1;	
	}

	close(fd);

	return 0;
}
