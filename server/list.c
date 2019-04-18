#include "list.h"

struct device_map_t device_map[64], *map;
int n_maps;

static void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	putc('\n', stderr);
}

static void load_device_list(void)
{
	int card, err;
	n_maps = 0;

	card = -1;
	if ((err = snd_card_next(&card)) < 0) {
		error("cannot determine card number: %s", snd_strerror(err));
		return;
	}
	if (card < 0) {
		error("no sound card found");
		return;
	}

	do
	{
		// list card devices
		snd_ctl_t *ctl;
		char name[32];
		int device;
		int err;

		sprintf(name, "hw:%d", card);
		if( (err = snd_ctl_open(&ctl, name, 0)) < 0 )
		{
			error("cannot open control for card %d: %s", card, snd_strerror(err));
			continue;
		}
		device = -1;
		for (;;)
		{
			if ((err = snd_ctl_rawmidi_next_device(ctl, &device)) < 0)
			{
				error("cannot determine device number: %s", snd_strerror(err));
				break;
			}
			if (device < 0)
				break;

			// list device
			snd_rawmidi_info_t *info;
			const char *name;
			const char *sub_name;
			int subs, subs_in, subs_out;
			int sub;
			int err;

			snd_rawmidi_info_alloca(&info);
			snd_rawmidi_info_set_device(info, device);

			snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
			err = snd_ctl_rawmidi_info(ctl, info);
			if (err >= 0)
				subs_in = snd_rawmidi_info_get_subdevices_count(info);
			else
				subs_in = 0;

			snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
			err = snd_ctl_rawmidi_info(ctl, info);
			if (err >= 0)
				subs_out = snd_rawmidi_info_get_subdevices_count(info);
			else
				subs_out = 0;

			subs = subs_in > subs_out ? subs_in : subs_out;
			if (!subs)
				continue;

			for (sub = 0; sub < subs; ++sub)
			{
				snd_rawmidi_info_set_stream(info,
					sub < subs_in ?
						SND_RAWMIDI_STREAM_INPUT :
						SND_RAWMIDI_STREAM_OUTPUT
				);
				snd_rawmidi_info_set_subdevice(info, sub);
				err = snd_ctl_rawmidi_info(ctl, info);
				if (err < 0)
				{
					error("cannot get rawmidi information %d:%d:%d: %s\n",
						  card, device, sub, snd_strerror(err));
					break;
				}

				map = &device_map[ n_maps ];
				name = snd_rawmidi_info_get_name(info);
				sub_name = snd_rawmidi_info_get_subdevice_name(info);
				if( sub == 0 && ( subs == 1 || sub_name[0] == '\0' ) )
				{
					sprintf(map->device, "hw:%d,%d", card, device);
					sprintf(map->name, "%s", name);
				}
				else
				{
					sprintf(map->device, "hw:%d,%d,%d", card, device, sub);
					sprintf(map->name, "%s", sub_name);
				}
				n_maps++;
			}
		}
		snd_ctl_close(ctl);
		if ((err = snd_card_next(&card)) < 0)
		{
			error("cannot determine card number: %s", snd_strerror(err));
			break;
		}
	}
	while (card >= 0);
}

void print_device_map()
{
	for(int i=0; i < n_maps; i++)
	{
		printf("%s\t%s\n", device_map[i].device, device_map[i].name);
	}
}

const char *find_device(const char *name)
{
	for(int i=0; i < n_maps; i++)
	{
		if( strstr( device_map[i].name, name ) != NULL )
		{
			return device_map[i].device;
		}
	}
	return "";
}

/*
void main( int argc, char **argv )
{
	load_device_list();
	//print_device_map();

	if( argc <= 1 )
	{
		error("usage: list <name> # search for hw by name");
		exit(1);
	}
	const char *name = argv[1];
	const char *dev = find_device(name);
	if( strlen(dev) > 0 )
	{
		printf("%s -> %s\n", name, dev);
		exit(0);
	}
	exit(ENODEV);
}
*/
