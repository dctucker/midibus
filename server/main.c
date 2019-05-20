// hw:JUNODS	hw:II	notes	3

#include "main.h"

void configure_midi_connection(const char *in_name, const char *out_name, const char *func_name, const char *args_name)
{
	//printf("R %s %s %s %s\n", in_name, out_name, func_name, args_name);
	int i = 0;
	for(; i < app->n_read_threads; ++i )
		if( strcmp( app->read_data[i].port_name, in_name ) == 0 )
			break;
	if( i == app->n_read_threads )
	{
		app->n_read_threads++;
		app->read_data[i].midi = NULL;
		app->read_data[i].port_name = in_name;
	}

	int o = 0;
	for(; o < app->read_data[i].n_outs; ++o )
		if( strcmp( app->read_data[i].outs[o].port_name, out_name ) == 0 )
			break;
	struct write_data *data = &app->read_data[i].outs[o];
	struct write_callback_t *callback;
	if( o == app->read_data[i].n_outs ) // new
	{
		clear_write_data( data );
		data->output_device = NULL;
		data->port_name = out_name;
		callback = setup_write_func( data, func_name );
		parse_write_args( callback, args_name );
		app->read_data[i].n_outs++;
	}
	else
	{
		callback = setup_write_func( data, func_name );
		parse_write_args( callback, args_name );
	}
}

void configure_connection(const char *in_name, const char *out_name, const char *func_name, const char *args_name)
{
	bool server_in  = strcmp( SERVER, in_name  ) == 0;
	bool server_out = strcmp( SERVER, out_name ) == 0;
	const char *port_name;

	if( server_in )
	{
		port_name = out_name;
		if( strcmp( func_name, "macro" ) == 0 )
		{
			setup_macro( port_name, args_name );
		}
	}
	else if( server_out )
	{
		port_name = in_name;
		if( strcmp( func_name, "macro" ) == 0 )
		{
			add_macro_listener( port_name, args_name );
		}
	}
	else
	{
		configure_midi_connection(in_name, out_name, func_name, args_name);
	}
}

void load_config_file()
{
	FILE *fp = fopen("midi-server.conf","r");
	printf("M midi-server.conf open\n");
	int n = 0;
	while( fscanf( fp, "%s\t%s\t%s\t%[^\n]", app->config[n].in, app->config[n].out, app->config[n].func, app->config[n].args) != EOF)
	{
		configure_connection(app->config[n].in, app->config[n].out, app->config[n].func, app->config[n].args);
		n++;
	}
	fclose(fp);
	printf("M midi-server.conf close\n");
}

void manage_inputs()
{
	int tries = 0;
	int had_errors;
	do
	{
		had_errors = 0;
		int i = 0;
		for(; i < app->n_read_threads; ++i )
		{
			if( app->read_data[i].midi != NULL )
				continue;
			if( strlen(app->read_data[i].port_name) <= 0 )
				continue;
			int result;
			result = setup_midi_device( &app->read_data[i] );
			if( result == 0 && app->read_data[i].midi != NULL )
			{
				pthread_create( &app->threads[i], NULL, read_thread, (void *) &app->read_data[i] );
			}
			else
			{
				had_errors++;
				if( tries == 4 )
				{
					printf("E %d %s \"%s\"\n", result, app->read_data[i].port_name, snd_strerror(result));
				}
			}
		}

		if( had_errors )
			usleep(400*1000);
		tries++;
	}
	while( had_errors && tries < 5 );
}

void manage_outputs()
{
	for( int i = 0; i < app->n_read_threads; ++i )
	{
		manage_thread_outputs( &app->read_data[i] );
	}
}

void join_threads()
{
	printf("M join threads\n");
	for( int i = 0; i < app->n_read_threads; ++i )
	{
		pthread_join( app->threads[i], NULL );
	}
}

void sighup_handler(int sig)
{
	static int hanging_up = 0;
	if( hanging_up )
		return;
	hanging_up = 1;

	printf("M Reloading\n");
	fflush(stdout);
	load_config_file();

	manage_inputs();
	manage_outputs();
	emit_devices();

	fflush(stdout);
	hanging_up = 0;
}

void sigint_handler(int sig)
{
	stop_all = 1;
	printf("\nM interrupt\n");
}

int main(int argc, char **argv)
{
	init_app();

	signal(SIGINT, sigint_handler);
	signal(SIGHUP, sighup_handler);
	//signal(SIGCHLD, sigchld_handler);

	pthread_t *socket_thread = setup_socket();
	load_config_file();

	manage_inputs();
	manage_outputs();
	printf("M idle loop\n");

	do // idle
		sleep(1);
	while( ! stop_all );

	join_threads();

	printf("M exit\n");

	stop_all = 1;
	pthread_join( *socket_thread, NULL );

	return EXIT_SUCCESS;
}
