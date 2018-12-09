#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <alsa/asoundlib.h>
#include <signal.h>

static snd_rawmidi_t *input, **inputp;
static snd_rawmidi_t *output, **outputp;

static void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	putc('\n', stderr);
}

int stop = 0;

void sig_handler(int sig)
{
	stop = 1;
}

int main(int argc, char **argv)
{
	int err;
	char *port_in  = "hw:JUNODS";
	char *port_out = "hw:Deluge";

	inputp = &input;
	outputp = NULL;
	if ((err = snd_rawmidi_open(inputp, outputp, port_in, SND_RAWMIDI_NONBLOCK)) < 0) {
		//error("cannot open port \"%s\": %s", port_in, snd_strerror(err));
		goto cleanup;
	}

	snd_rawmidi_read(input, NULL, 0); // trigger reading

	int read = 0;
	int npfds, time = 0;
	struct pollfd *pfds;

	npfds = snd_rawmidi_poll_descriptors_count(input);
	pfds = alloca(npfds * sizeof(struct pollfd));
	snd_rawmidi_poll_descriptors(input, pfds, npfds);
	signal(SIGINT, sig_handler);

	do
	{
		int i;
		unsigned short revents;
		unsigned char buf[1024];

		err = poll(pfds, npfds, 200);
		if (err < 0) {
			if( ! stop )
				error("poll failed: %s", strerror(errno));
			break;
		}
		if ((err = snd_rawmidi_poll_descriptors_revents(input, pfds, npfds, &revents)) < 0) {
			error("cannot get poll events: %s", snd_strerror(errno));
			break;
		}
		if( revents & (POLLERR | POLLHUP ) ) break;
		if( ! ( revents & POLLIN ) ) continue;
		err = snd_rawmidi_read( input, buf, sizeof(buf) );
		if (err == -EAGAIN) continue;
		if (err < 0) {
			error("cannot read from port \"%s\": %s", port_in, snd_strerror(err));
			break;
		}
		for (i = 0; i < err; ++i)
			printf("%02X ", buf[i]);
		printf("\n");
		fflush(stdout);
	}
	while( ! stop );

cleanup:
	if (inputp)
		snd_rawmidi_close(input);
	if (outputp)
		snd_rawmidi_close(output);
	printf("\n");
	fflush(stderr);
	fflush(stdout);
}
