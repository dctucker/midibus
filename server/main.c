#include "write.h"
#include "thru.h"
#include "main.h"

void configure(const char *connections[][3], size_t n_connections)
{
	for( int j = 0; j < n_connections; ++j )
	{
		int i = 0;
		for(; i < n_read_threads; ++i )
			if( strcmp( read_data[i].port_name, connections[j][0] ) == 0 )
				break;

		if( i == n_read_threads )
		{
			n_read_threads++;
			read_data[i].port_name = connections[j][0];
		}

		int o = read_data[i].n_outs;
		read_data[i].outs[o].midi      = NULL;
		read_data[i].outs[o].port_name = connections[j][1];
		read_data[i].outs[o].func      = write_func( connections[j][2] );
		read_data[i].n_outs++;
	}
}

void manage_outputs()
{
	for( int i = 0; i < n_read_threads; ++i )
	{
		manage_thread_outputs( &read_data[i] );
	}
}

void sighup_handler(int sig)
{
	manage_outputs();
}

void sigint_handler(int sig)
{
	stop_all = 1;
}

int main(int argc, char **argv)
{
	pthread_t threads[MAX_THREADS];
	stop_all = 0;

	const char *config[][3] = {
		{"hw:JUNODS", "hw:Deluge", "thru"}
	};
	configure(config, 1);

	int i = 0;

	signal(SIGINT, sigint_handler);
	signal(SIGHUP, sighup_handler);
	pthread_create( &threads[i], NULL, read_thread, (void *) &read_data[0] );
	sleep(1);
	pthread_join( threads[i], NULL);
	return EXIT_SUCCESS;
}
