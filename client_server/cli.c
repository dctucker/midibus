#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

//char *socket_path = "./socket";
char *socket_path = "\0hidden";

fd_set readfds;
int max_sd;
int activity;

int main(int argc, char *argv[]) {
	struct sockaddr_un addr;
	char buffer[1025];
	int server_socket, read_count;

	if (argc > 1) socket_path=argv[1];

	if ( (server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (*socket_path == '\0') {
		*addr.sun_path = '\0';
		strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
	} else {
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
	}

	if (connect(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	while( 1 )
	{
		// setup our file descriptors to be selected
		FD_ZERO( &readfds );
		FD_SET( server_socket, &readfds );
		FD_SET( STDIN_FILENO , &readfds );
		max_sd = max(server_socket, STDIN_FILENO);

		activity = select( max_sd + 1, &readfds, NULL, NULL, NULL );
		if( activity < 0 && errno != EINTR )
		{
			printf("select error\n");
		}

		if( FD_ISSET( STDIN_FILENO, &readfds ) )
		{
			read_count = read(STDIN_FILENO, buffer, sizeof(buffer));
			if( write(server_socket, buffer, read_count) != read_count )
			{
				if (read_count > 0)
					fprintf(stderr,"partial write");
				else {
					perror("write error");
					exit(EXIT_FAILURE);
				}
			}
		}
		
		if( FD_ISSET( server_socket, &readfds ) )
		{
			read_count = read( server_socket, buffer, 1024 );
			if( read_count == 0 )
				break; // disconnect

			buffer[ read_count ] = '\0';
			printf("%s\n", buffer);

		}
	}
	close(server_socket);

	return 0;
}
