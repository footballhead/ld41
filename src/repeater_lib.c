#include "repeater_lib.h"

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

// Piggy pack off of the pseudoterminal system to inject messages into the
// player's window. Hopefully, this is the only terminal the player will ahve...
#define REPEATER_FILE "/dev/pts/0"

int print_message_to_player(const char* message)
{
	int fd = -1;
	int len = strlen(message);

	fd = open(REPEATER_FILE, O_RDWR);
	if (fd == -1) {
		perror("open failed");
		return -1;
	}

	if (write(fd, message, len) != len) {
		perror("write failed");
		close(fd);
		return -1;	
	}

	close(fd);

	fprintf(stderr, "DEBUG: %s", message);

	return 0;
}
