#include "app.h"

void manage_thread_outputs(struct read_thread_data *in)
{
	if( in->midi == NULL )
		return;
	for( int o = 0; o < in->n_outs; ++o )
	{
		int err;
		struct write_data *out = &in->outs[o];

		if( out->output_device != NULL )
			continue;

		// find named output in output_devices
		int d = 0;
		for(; d < app.n_output_devices; ++d )
		{
			if( strcmp( app.output_devices[d].port_name, out->port_name ) == 0 )
			{
				if( app.output_devices[d].midi != NULL )
				{
					app.output_devices[d].midi_in_exclusive = NULL;
					out->midi_in = in->midi;
					out->output_device = &app.output_devices[d];
					printf("C %s -> %s\n", in->port_name, out->port_name);
				}
				break;
			}
		}
		/*
		// in list but not opened yet
		if( app.output_devices[d].midi == NULL )
		{
			if( (err = snd_rawmidi_open(NULL, &out->midi, out->port_name, SND_RAWMIDI_APPEND | SND_RAWMIDI_NONBLOCK)) < 0 )
			{
				error("cannot open port \"%s\": %s", out->port_name, snd_strerror(err));
				continue;
			}
			printf("Opened %s for write\n", out->port_name);
		}
		*/
	}
}

int setup_midi_device(struct read_thread_data *data)
{
	int err;
	snd_rawmidi_t *midi_out;

	if ((err = snd_rawmidi_open(&data->midi, &midi_out, data->port_name, SND_RAWMIDI_APPEND | SND_RAWMIDI_NONBLOCK)) < 0) {
		//error("E %d %s \"%s\"", err, data->port_name, snd_strerror(err));
		data->midi = NULL;
		return err;
	}
	printf("S %s\n", data->port_name);
	int d = 0;
	for(; d < app.n_output_devices; ++d )
		if( strcmp( app.output_devices[d].port_name, data->port_name ) == 0 )
			break;
	if( d == app.n_output_devices )
	{
		app.output_devices[d].port_name = data->port_name;
		app.n_output_devices++;
	}
	app.output_devices[d].midi = midi_out;
	manage_thread_outputs( data );
	return 0;
}

void *read_thread(void *arg)
{
	int err;
	struct read_thread_data *data = arg;
	snd_rawmidi_read(data->midi, NULL, 0); // trigger reading

	int read = 0;
	int npfds, time = 0;
	struct pollfd *pfds;

	npfds = snd_rawmidi_poll_descriptors_count(data->midi);
	pfds = alloca(npfds * sizeof(struct pollfd));
	snd_rawmidi_poll_descriptors(data->midi, pfds, npfds);

	printf("T %s begin\n", data->port_name);
	do
	{
		int i;
		unsigned short revents;
		unsigned char buf[BUFSIZE];

		err = poll(pfds, npfds, 200);
		if (err < 0) {
			if( ! stop_all )
				error("E %d %s \"poll failed: %s\"", errno, data->port_name, strerror(errno));
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
		if (err < 0)
		{
			error("E %d %s \"%s\"", err, data->port_name, snd_strerror(err));
			break;
		}

		printf("I ");
		for (i = 0; i < err; ++i)
			printf("0x%02X ", buf[i]);
		printf("%s\n", data->port_name);

		for( int o = 0; o < data->n_outs; ++o )
		{
			struct write_data *out = &data->outs[o];
			if( out->output_device == NULL || out->output_device->midi == NULL || out->callbacks == NULL )
				continue;
			snd_rawmidi_t *excl = out->output_device->midi_in_exclusive;

			if( excl != NULL && excl != data->midi )
				continue;

			for( int c = 0; c < MAX_CALLBACKS; c++ )
			{
				struct write_callback_t *callback = &out->callbacks[c];
				if( callback->func == NULL )
					continue;
				int res = callback->func( out, &(callback->args), buf, err, &(callback->out_buf) );
				if( res > 0 )
					printf("%s\n", out->port_name);
				else if( res < 0 )
					break;
			}
		}
		fflush(stdout);
	}
	while( ! stop_all );

cleanup:
	if( data->midi )
	{
		snd_rawmidi_close(data->midi);
		printf("X %s\n", data->port_name);
		data->midi = NULL;
	}

	for( int o = 0; o < data->n_outs; ++o )
	{
		struct write_data *out = &data->outs[o];
		printf("X %s /> %s", data->port_name, out->port_name);
		teardown_write_func( out );
		printf("\n");
	}

	printf("T %s end\n", data->port_name);
	fflush(stderr);
	fflush(stdout);
}
