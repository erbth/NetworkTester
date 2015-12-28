#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <inttypes.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define READ_BUF_SIZE 1000000


int main (int argc, char **argv)
{
	int error = EXIT_SUCCESS;
	fd_set fds, readFds;
	int maxFd;
	char buf[30];
	uint8_t *readBuf = NULL;
	socklen_t size;

	int listener = -1, connection = -1;
	struct sockaddr_in addr;

	readBuf = malloc (READ_BUF_SIZE * sizeof (uint8_t));
	if (!readBuf)
	{
		perror ("couldn't allocate memory");
		error = EXIT_FAILURE;
		goto END;
	}

	listener = socket (AF_INET, SOCK_STREAM, 0);
	if (listener < 0)
	{
		perror ("couldn't create socket");
		error = EXIT_FAILURE;
		goto END;
	}

	memset (&addr, 0, sizeof (addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons (10001);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind (listener, (struct sockaddr *) &addr, sizeof (addr)) < 0)
	{
		perror ("couldn't bind socket");
		error = EXIT_FAILURE;
		goto END;
	}


	if (listen (listener, 5) < 0)
	{
		perror ("couldn't start listening");
		error = EXIT_FAILURE;
		goto END;
	}


	for (;;)
	{
		printf ("waiting for incomming connection ...\n");

		size = sizeof (struct sockaddr_in);

		connection = accept (listener, (struct sockaddr *) &addr, &size);
		if (connection < 0)
		{
			perror ("couldn't accept connection");
			error = EXIT_FAILURE;
			goto END;
		}

		printf ("accepted connection from %s\n", inet_ntop (AF_INET, &addr.sin_addr, buf, 30));

		FD_ZERO (&fds);
		FD_SET (connection, &fds);

		readFds = fds;

		maxFd = connection;

		while (select (maxFd + 1, &readFds, NULL, NULL, NULL) > 0)
		{
			if (FD_ISSET (connection, &readFds))
			{
				if (read (connection, readBuf, READ_BUF_SIZE) <= 0)
					break;
			}

			readFds = fds;
		}

		printf ("  droped connection\n");
	}

END:
	if (listener >= 0)
		close (listener);

	if (connection >= 0)
		close (connection);

	if (readBuf)
		free (readBuf);

	return error;
}
