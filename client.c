#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define WRITE_BUFFER_SIZE 1000000


int main (int argc, char **argv)
{
	int error = EXIT_SUCCESS;
	int ret;
	fd_set writeFds, fds, readFds, rfds;
	int maxFd;
	uint8_t *writeBuffer = NULL;

	int connection = -1;
	struct sockaddr_in addr;
	struct addrinfo hints, *addresses, *currentAddress;

	if (argc != 2)
	{
		printf ("usage: %s <host>\n", argv[0]);
		goto END;
	}

	writeBuffer = malloc (WRITE_BUFFER_SIZE);
	if (!writeBuffer)
	{
		perror ("couldn't allocate memory\n");
		error = EXIT_FAILURE;
		goto END;
	}

	memset (writeBuffer, 0, WRITE_BUFFER_SIZE);

	connection = socket (AF_INET, SOCK_STREAM, 0);
	if (connection < 0)
	{
		perror ("couldn't create socket");
		error = EXIT_FAILURE;
		goto END;
	}

	memset (&addr, 0, sizeof (addr));


	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((ret = getaddrinfo (argv[1], "10001", &hints, &addresses) == 0))
	{
		currentAddress = addresses;

		while (currentAddress)
		{
			if (connect (connection, currentAddress->ai_addr, sizeof (struct sockaddr)) == 0)
			{
				ret = 1;
				break;
			}

			currentAddress = currentAddress->ai_next;
		}

		freeaddrinfo (addresses);

		if (ret != 1)
		{
			perror ("couldn't connect to host");
			error = EXIT_FAILURE;
			goto END;
		}
	}
	else
	{
		fprintf (stderr, "getaddrinfo returned %d: %s\n", ret, gai_strerror (ret));
		error = EXIT_FAILURE;
		goto END;
	}

	FD_ZERO (&fds);
	FD_SET (connection, &fds);

	FD_ZERO (&rfds);
	FD_SET (STDIN_FILENO, &rfds);
	
	writeFds = fds;
	readFds = rfds;

	maxFd = STDIN_FILENO;

	if (connection > STDIN_FILENO)
		maxFd = connection;

	printf ("now writing to host, quit with 'q'\n");

	while (select (maxFd + 1, &rfds, &fds, NULL, NULL) > 0)
	{
		if (FD_ISSET (connection, &fds))
		{
			if (write (connection, writeBuffer, WRITE_BUFFER_SIZE) <= 0)
			{
				perror ("couldn't write to socket\n");
				error = EXIT_FAILURE;
				goto END;
			}
		}

		if (FD_ISSET (STDIN_FILENO, &rfds))
		{
			if (getchar () == 'q')
			{
				getchar ();
				break;
			}

			getchar ();
		}

		fds = writeFds;
		rfds = readFds;
	}


END:
	if (connection >= 0)
		close (connection);

	if (writeBuffer)
		free (writeBuffer);

	return error;
}
