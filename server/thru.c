#include "thru.h"

static void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	putc('\n', stderr);
}

void sig_handler(int sig)
{
	stop_all = 1;
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

void manage_outputs()
{
	printf("Managing outputs\n");
	for( int i = 0; i < MAX_THREADS; ++i )
	{
		struct read_thread_data *in = &read_data[i];
		if( in->midi == NULL )
			continue;
		printf("Checking input %d\n", i);
		for( int o = 0; o < in->n_outs; ++o )
		{
			int err;
			struct write_data *out = &in->outs[o];

			if( out->midi != NULL ) continue;
			if ((err = snd_rawmidi_open(NULL, &out->midi, out->port_name, SND_RAWMIDI_NONBLOCK)) < 0) {
				error("cannot open port \"%s\": %s", out->port_name, snd_strerror(err));
				continue;
			}
			printf("Connected %s to %s\n", in->port_name, out->port_name);
		}
	}
}

int main(int argc, char **argv)
{
	pthread_t threads[MAX_THREADS];

	read_data[0].port_name = "hw:JUNODS";
	read_data[0].outs[0].port_name = "hw:Deluge";
	read_data[0].outs[0].midi = NULL;
	read_data[0].outs[0].func = write_thru;
	read_data[0].n_outs = 1;

	int i = 0;

	signal(SIGINT, sig_handler);
	signal(SIGHUP, manage_outputs);
	pthread_create( &threads[i], NULL, read_thread, (void *) &read_data[0] );
	sleep(1);
	manage_outputs();
	pthread_join( threads[i], NULL);
	return EXIT_SUCCESS;
}

