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
	printf("\nClosed %s\n", data->port_name);
	fflush(stderr);
	fflush(stdout);
}
