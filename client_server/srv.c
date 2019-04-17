#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <errno.h>

//char *socket_path = "./socket";
char *socket_path = "\0hidden";
#define MAX_CLIENTS 16

struct sockaddr_un address;
char buffer[1025];
size_t addrlen;
int master_socket, client_socket, read_count;
int client_sockets[MAX_CLIENTS];
int activity;
int max_sd;
fd_set readfds;

void send_all(const char *message)
{
	int len = strlen(message);
	for( int i = 0; i < MAX_CLIENTS; i++ )
	{
		if( client_sockets[i] == 0 )
			continue;
		send( client_sockets[i], message, len, 0 );
	}
}

int main(int argc, char *argv[]) {

	if (argc > 1) socket_path=argv[1];

	for( int i = 0; i < MAX_CLIENTS; i++ )
		client_sockets[i] = 0;

	if ( (master_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(master_socket);
	}
	/*
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	*/
	/*
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	*/

	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	if (*socket_path == '\0')
	{
		*address.sun_path = '\0';
		strncpy(address.sun_path+1, socket_path+1, sizeof(address.sun_path)-2);
	}
	else
	{
		strncpy(address.sun_path, socket_path, sizeof(address.sun_path)-1);
		unlink(socket_path);
	}

	if (bind(master_socket, (struct sockaddr*) &address, sizeof(address)) < 0)
	{
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if (listen(master_socket, 5) == -1)
	{
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	addrlen = sizeof(address);   

	while (1)
	{
		// setup our file descriptors to be selected
		FD_ZERO( &readfds );
		FD_SET( master_socket, &readfds );
		max_sd = master_socket;
		for( int i = 0; i < MAX_CLIENTS; i++ )
		{
			client_socket = client_sockets[i];
			if( client_socket > 0 )
				FD_SET( client_socket, &readfds );

			if( client_socket > max_sd )
				max_sd = client_socket;
		}

		activity = select( max_sd + 1, &readfds, NULL, NULL, NULL );
		if( activity < 0 && errno != EINTR )
		{
			printf("select error\n");
		}

		// check for activity on master for new connections
		if( FD_ISSET( master_socket, &readfds ) )
		{
			if( (client_socket = accept( master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0 )
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			printf("client connected\n");

			// add newly connected socket to array
			for( int i = 0; i < MAX_CLIENTS; i++ )
			{
				if( client_sockets[i] == 0 )
				{
					client_sockets[i] = client_socket;
					break;
				}
			}
		}

		// send and receive for each client with activity
		for( int i = 0; i < MAX_CLIENTS; i++ )
		{
			client_socket = client_sockets[i];
			if( FD_ISSET( client_socket, &readfds ) )
			{
				if( (read_count = read( client_socket, buffer, 1024 )) == 0 )
				{
					// disconnected
					printf("client disconnected\n");
					close( client_socket );
					client_sockets[i] = 0;
				}
				else
				{
					// deal with received buffer
					buffer[ read_count ] = '\0';
					printf("read %u bytes: %.*s\n",  read_count,  read_count, buffer);
					send_all( buffer );
				}
			}
		}
	}

	return 0;
}
