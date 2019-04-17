#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "socket.h"
#include "common.h"
#include "thru.h"

pthread_t thread;
struct sockaddr_in servaddr, cliaddr;
int clilen;

void setup_socket()
{
	int err;
	if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		printf("E socket creation failed\n");
		return;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(UDP_PORT);
	if( (err = bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr))) < 0 )
	{
		printf("E %d UDP bind failed\n", err);
		return;
	}
	printf("U %d bound\n", UDP_PORT);

	pthread_create( &thread, NULL, socket_thread, NULL );
	//pthread_join( thread, NULL );
}

void sendto_client(const char *str)
{
	sendto(sockfd, str, strlen(str), MSG_CONFIRM, ( struct sockaddr *) &cliaddr, clilen);
}

void send_config()
{
	char out_buffer[ BUFSIZE ];
	sendto_client("config\n");
	for( int i = 0; i < n_read_threads; ++i )
	{
		for( int o = 0; o < read_data[i].n_outs; ++o )
		{
			sprintf(out_buffer, "%s\t%s\t%s\t%s\n",
				read_data[i].port_name,
				read_data[i].outs[o].port_name,
				read_data[i].outs[o].func_name,
				read_data[i].outs[o].args_name
			);
			sendto_client(out_buffer);
		}
	}
	sendto_client("\003");
}

void send_devices()
{
	char out_buffer[ BUFSIZE ];
	sendto_client("devices\n");
	for( int i = 0; i < n_read_threads; ++i )
	{
		if( read_data[i].midi != NULL )
		{
			sprintf(out_buffer, "%s\n", read_data[i].port_name);
			sendto_client(out_buffer);
		}
	}
	sendto_client("\003");
}

void *socket_thread()
{
	char buffer[ BUFSIZE ];
	char out_buffer[ 16 * BUFSIZE ];
	int num_bytes;
	printf("U thread started\n");
	do
	{
		memset(&cliaddr, 0, sizeof(cliaddr));
		clilen = sizeof(cliaddr);
		num_bytes = recvfrom(sockfd, buffer, BUFSIZE, 0, ( struct sockaddr *) &cliaddr, &clilen);
		buffer[num_bytes] = '\000';
		printf("U %d %s", htons(cliaddr.sin_port), buffer);
		if( strcmp( buffer, "config\n" ) == 0 )
		{
			send_config();
		}
		else if( strcmp( buffer, "devices\n" ) == 0 )
		{
			send_devices();
		}
	}
	while( ! stop_all );
	close(sockfd);
	printf("U close\n");
}
