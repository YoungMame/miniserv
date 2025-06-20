#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>


const int BUFFER_SIZE = 400000;
const int MAX_CLIENT_SIZE = 5000;

static int g_is_running = 1;
int g_client_fds[MAX_CLIENT_SIZE];
int last_client_attributed = 0;

void broadcast(char *message, int except)
{
	int	i = 0;
	for (int i = 0; i < MAX_CLIENT_SIZE; i++) {
		if (g_client_fds[i] != 0 && g_client_fds[i] != except)
			send(g_client_fds[i], message, strlen(message), 0);
	}
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void ft_putstr(char *str) // Print
{
	int i = 0;

	while (str && str[i])
	{
		write(1, &(str[i]), 1);
		i++;
	}
}

void exit_fatal()
{
	ft_putstr("Fatal Error\n");
	exit(1);
}


int main() {
	int maxfd;
	fd_set readfds, writefds;
	int server_socket; // server socket File Descriptor
	struct sockaddr_in server_address;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	FD_SET(0, &readfds);
	maxfd = server_socket;
	FD_SET(server_socket, &readfds);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(4045); 
	server_address.sin_addr.s_addr = INADDR_ANY;

	char received_message[BUFFER_SIZE];
	char message_to_send[BUFFER_SIZE];

	if (bind(server_socket, (const struct sockaddr *)&server_address, sizeof(server_address)) == -1)
		return (exit_fatal(), 1);
	printf("Socket is binded with address\n");
	if (listen(server_socket, 42) == -1)
		return (exit_fatal(), 1);
	printf("Socket is listening:\n");

	while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(server_socket, &readfds);
		for (int i = 0; i < 5000; i++)
		{
			if (g_client_fds[i] > 0)
				FD_SET(g_client_fds[i], &readfds);
		}

		int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
		if (activity == -1)
			return (exit_fatal(), 1);
		printf("An fd selected something...\n");

		if (FD_ISSET(server_socket, &readfds))
		{
			printf("Server socket...\n");
			socklen_t len = sizeof(server_address);
			int clientfd = accept(server_socket, (struct sockaddr *)&server_address, &len);
			if (clientfd == -1)
				return (exit_fatal(), 1);
			g_client_fds[last_client_attributed] = clientfd;
			last_client_attributed++;
			if (clientfd > maxfd)
				maxfd = clientfd;
			printf("client fd = %i\n", clientfd);
		}
		else
		{
			for (int i = 0; i < 5000; i++)
			{
				int fd = g_client_fds[i];
				if (fd > 0 && FD_ISSET(fd, &readfds))
				{
					bzero(received_message, BUFFER_SIZE);
					bzero(message_to_send, BUFFER_SIZE);
					int bytes = recv(fd, received_message, sizeof(received_message) - 1, 0);
					if (bytes <= 0)
					{
						sprintf(message_to_send, "client %i leaved", i);
						broadcast(message_to_send, i);
						FD_CLR(fd, &writefds);
						FD_CLR(fd, &readfds);
						g_client_fds[i] = 0;
						close(fd);
					}
					else
					{
						printf("Message received: %s\n", received_message);
						sprintf(message_to_send, "client %i : %s", i, received_message);
						broadcast(message_to_send, i);
					}
				}
			}
		}
	}
}