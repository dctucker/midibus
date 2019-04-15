#include "socket.h"
#include "write.h"
#include "thru.h"
#include "main.h"

void configure_connection(const char *in_name, const char *out_name, const char *func_name, const char *args)
{
	printf("%s %s %s %s\n", in_name, out_name, func_name, args);
	int i = 0;
	for(; i < n_read_threads; ++i )
		if( strcmp( read_data[i].port_name, in_name ) == 0 )
			break;
	if( i == n_read_threads )
	{
		n_read_threads++;
		read_data[i].midi = NULL;
		read_data[i].port_name = in_name;
	}

	int o = 0;
	for(; o < read_data[i].n_outs; ++o )
		if( strcmp( read_data[i].outs[o].port_name, out_name ) == 0 )
			break;
	if( o == read_data[i].n_outs )
	{
		struct write_data *data = &read_data[i].outs[o];
		data->output_device = NULL;
		data->port_name = out_name;
		data->func_name = func_name;
		data->args_name = args;
		setup_write_func( data );
		read_data[i].n_outs++;
	}
}

void manage_inputs()
{
	int tries = 0;
	int had_errors;
	do
	{
		had_errors = 0;
		int i = 0;
		for(; i < n_read_threads; ++i )
		{
			if( read_data[i].midi != NULL )
				continue;
			if( strlen(read_data[i].port_name) <= 0 )
				continue;
			int result;
			result = setup_midi_device( &read_data[i] );
			if( result == 0 && read_data[i].midi != NULL )
				pthread_create( &threads[i], NULL, read_thread, (void *) &read_data[i] );
			else
				had_errors++;
		}

		if( had_errors )
			usleep(400*1000);
		tries++;
	}
	while( had_errors && tries < 5 );
}

void manage_outputs()
{
	for( int i = 0; i < n_read_threads; ++i )
	{
		manage_thread_outputs( &read_data[i] );
	}
}

void load_config_file()
{
	const char in[MAX_CONNECTIONS][MAX_STRING], out[MAX_CONNECTIONS][MAX_STRING];
	const char func[MAX_CONNECTIONS][MAX_STRING], args[MAX_CONNECTIONS][MAX_STRING];
	FILE *fp = fopen("midi-server.conf","r");
	int n = 0;
	while( fscanf( fp, "%s %s %s %s", &in[n], &out[n], &func[n], &args[n]) != EOF)
	{
		configure_connection(in[n], out[n], func[n], args[n]);
		n++;
	}
	fclose(fp);
}

void sighup_handler(int sig)
{
	static int hanging_up = 0;
	if( hanging_up )
		return;
	hanging_up = 1;

	printf("Reloading\n");
	fflush(stdout);
	load_config_file();
	manage_inputs();
	manage_outputs();
	fflush(stdout);
	hanging_up = 0;
}

void sigint_handler(int sig)
{
	stop_all = 1;
}

int main(int argc, char **argv)
{
	stop_all = 0;

	load_config_file();

	signal(SIGINT, sigint_handler);
	signal(SIGHUP, sighup_handler);

	manage_inputs();
	manage_outputs();
	setup_socket();

	do // idle
		sleep(1);
	while( ! stop_all );

	printf("Done.\n");

	return EXIT_SUCCESS;
}
