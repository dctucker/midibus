#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"
#include "common.h"
#include "thru.h"
#include "app.h"

pthread_t thread;

//struct sockaddr_un address;
struct sockaddr_in address;
size_t addrlen;
int master_socket, client_socket, read_count;
int client_sockets[MAX_CLIENTS];
int activity;
int max_sd;
fd_set readfds;

char out_buffer[ 4096 ];

pthread_t *setup_socket()
{
	pthread_create( &thread, NULL, socket_thread, NULL );
	//pthread_join( thread, NULL );
	return &thread;
}

void send_all(const char *message)
{
	int len = strlen(message);
	for( int i = 0; i < MAX_CLIENTS; i++ )
	{
		if( client_sockets[i] == 0 )
			continue;
		send( client_sockets[i], message, len, 0 );
		printf("N send %s\n", message);
	}
}

void emit_config()
{
	char *ptr = &out_buffer[0];

	ptr += sprintf(ptr, "config");
	for( int i = 0; i < app.n_read_threads; ++i )
	{
		for( int o = 0; o < app.read_data[i].n_outs; ++o )
		{
			ptr += sprintf(ptr, "\n%s\t%s\t%s\t%s",
				app.read_data[i].port_name,
				app.read_data[i].outs[o].port_name,
				app.read_data[i].outs[o].func_name,
				app.read_data[i].outs[o].args_name
			);
		}
	}
	*ptr = '\0';
	send_all(out_buffer);
}

void emit_devices()
{
	char *ptr = &out_buffer[0];

	ptr += sprintf(ptr, "devices");
	for( int i = 0; i < app.n_read_threads; ++i )
	{
		if( app.read_data[i].midi != NULL )
		{
			ptr += sprintf(ptr, "\n%s", app.read_data[i].port_name);
		}
	}
	*ptr = '\0';
	send_all(out_buffer);
}

void *socket_thread()
{
	char buffer[ BUFSIZE ];
	int num_bytes;
	printf("N thread started\n");

	for( int i = 0; i < MAX_CLIENTS; i++ )
		client_sockets[i] = 0;

	if ( (master_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(master_socket);
	}

	//memset(&address, 0, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	/*
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
	*/

	if( bind(master_socket, (struct sockaddr*) &address, sizeof(address)) < 0 )
	{
		perror("E bind error");
		exit(EXIT_FAILURE);
	}

	if( listen(master_socket, 5) < 0 )
	{
		perror("E listen error");
		exit(EXIT_FAILURE);
	}

	addrlen = sizeof(address);   

	do
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
			printf("E select error\n");
		}

		// check for activity on master for new connections
		if( FD_ISSET( master_socket, &readfds ) )
		{
			if( (client_socket = accept( master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0 )
			{
				perror("E accept");
				exit(EXIT_FAILURE);
			}

			printf("N client connected\n");

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
				read_count = read( client_socket, buffer, BUFSIZE );
				if( read_count == 0 )
				{
					// disconnected
					printf("N client disconnected\n");
					close( client_socket );
					client_sockets[i] = 0;
				}
				else if( read_count < 0 )
				{
					printf("E client error %d\n", read_count);
				}
				else
				{
					// deal with received buffer
					buffer[ read_count ] = '\0';

					printf("N request %s", buffer);
					if( strcmp( buffer, "config\n" ) == 0 )
					{
						emit_config();
					}
					else if( strcmp( buffer, "devices\n" ) == 0 )
					{
						emit_devices();
					}
					else if( strcmp( buffer, "bye\n" ) == 0 )
					{
						printf("N client disconnected\n");
						close( client_socket );
						client_sockets[i] = 0;
					}
					printf("N response sent\n");
				}
			}
		}
	}
	while( ! stop_all );

	printf("N thread ending\n");
	for( int i = 0; i < MAX_CLIENTS; i++ )
	{
		close( client_sockets[i] );
		client_sockets[i] = 0;
	}
	close( master_socket );
	printf("N close\n");
}
