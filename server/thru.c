#include "thru.h"

static void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	putc('\n', stderr);
}

void manage_thread_outputs(struct read_thread_data *in)
{
	if( in->midi == NULL )
		return;
	for( int o = 0; o < in->n_outs; ++o )
	{
		int err;
		struct write_data *out = &in->outs[o];

		if( out->midi != NULL ) continue;

		// find named output in output_devices
		int d = 0;
		for(; d < n_output_devices; ++d )
			if( strcmp( output_devices[d].port_name, out->port_name ) == 0 )
				break;
		// not found, must be new
		if( d == n_output_devices )
		{
			output_devices[d].midi = NULL;
			output_devices[d].port_name = out->port_name;
			n_output_devices++;
		}
		// in list but not opened yet
		if( output_devices[d].midi == NULL )
		{
			if( (err = snd_rawmidi_open(NULL, &out->midi, out->port_name, SND_RAWMIDI_NONBLOCK)) < 0 )
			{
				error("cannot open port \"%s\": %s", out->port_name, snd_strerror(err));
				continue;
			}
		}
		out->midi = output_devices[d].midi;
		printf("Connected %s to %s\n", in->port_name, out->port_name);
	}
}

void manage_outputs()
{
	for( int i = 0; i < n_read_threads; ++i )
	{
		manage_thread_outputs( &read_data[i] );
	}
}

void *read_thread(void *arg)
{
	int err;
	struct read_thread_data *data = arg;

	if ((err = snd_rawmidi_open(&data->midi, NULL, data->port_name, SND_RAWMIDI_NONBLOCK)) < 0) {
		error("cannot open port \"%s\": %s", data->port_name, snd_strerror(err));
		goto cleanup;
	}
	printf("Opened %s for read\n", data->port_name);
	manage_thread_outputs( data );

	snd_rawmidi_read(data->midi, NULL, 0); // trigger reading

	int read = 0;
	int npfds, time = 0;
	struct pollfd *pfds;

	npfds = snd_rawmidi_poll_descriptors_count(data->midi);
	pfds = alloca(npfds * sizeof(struct pollfd));
	snd_rawmidi_poll_descriptors(data->midi, pfds, npfds);

	do
	{
		int i;
		unsigned short revents;
		unsigned char buf[1024];

		err = poll(pfds, npfds, 200);
		if (err < 0) {
			if( ! stop_all )
				error("poll failed: %s", strerror(errno));
			break;
		}
		if ((err = snd_rawmidi_poll_descriptors_revents(data->midi, pfds, npfds, &revents)) < 0) {
			error("cannot get poll events: %s", snd_strerror(errno));
			break;
		}
		if( revents & (POLLERR | POLLHUP ) ) break;
		if( ! ( revents & POLLIN ) ) continue;
		err = snd_rawmidi_read( data->midi, buf, sizeof(buf) );
		if (err == -EAGAIN) continue;
		if (err < 0) {
			error("cannot read from port \"%s\": %s", data->port_name, snd_strerror(err));
			break;
		}

		for( int o = 0; o < data->n_outs; ++o )
		{
			struct write_data *out = &data->outs[o];
			if( out->midi == NULL || out->func == NULL )
				continue;
			out->func( out->midi, buf, err );
		}

		for (i = 0; i < err; ++i)
			printf("%02X ", buf[i]);
		printf("\n");
		fflush(stdout);
	}
	while( ! stop_all );

cleanup:
	snd_rawmidi_close(data->midi);
	printf("\nDone.\n");
	fflush(stderr);
	fflush(stdout);
}

int write_thru(snd_rawmidi_t *port, unsigned char *buf, int n_bytes)
{
	snd_rawmidi_write( port, buf, n_bytes );
}

int write_none(snd_rawmidi_t *port, unsigned char *buf, int n_bytes)
{
	return 0;
}

int (*write_func(const char *name))()
{
	if( strcmp(name, "thru") == 0);
		return write_thru;
	return write_none;
}

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

