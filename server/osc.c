#include "common.h"
#include "app.h"
#include "osc.h"

pthread_t osc_thread;

lo_server_thread osc_server;
struct config_line osc_config[256];
struct osc_map osc_maps[256];
int num_maps = 0;

void osc_error(int num, const char *msg, const char *path)
{
	printf("E osc liblo server error %d in path %s:\n", num, path, msg);
	fflush(stdout);
}

int osc_default_handler(const char *path, const char *types, lo_arg **argv, int argc, void *msg, void *user_data)
{
	int i;
	printf("N osc %s ", path);
	for (i = 0; i < argc; i++) {
		printf(" %c:", types[i]);
		lo_arg_pp((lo_type)types[i], argv[i]);
	}
	printf("\n");
	fflush(stdout);
	return 1;
}

int osc_cc_handler(const char *path, const char *types, lo_arg **argv, int argc, void *msg, void *user_data)
{
	struct osc_map *map = user_data;
	float arg = argv[0]->f;
	//printf("V port=%s cc=%d f=%f\n", map->port_name, map->cc, arg);
	int value;
	if( map->values[0] == 0xff )
	{
		value = (int)(arg * 127.0);
	}
	else
	{
		char *end = strrchr(path, '/');
		int base = atoi( end + 1 );
		value = map->values[base];
	}

	char data[6];
	size_t n_data = 3;
	data[0] = 0xb0 | ( map->ch & 0x0f );
	data[1] = map->cc & 0x7f;
	data[2] = value & 0x7f;

	int o = 0;
	for(; o < app->n_output_devices; ++o )
		if( app->output_devices[o].port_name != NULL && strcmp( app->output_devices[o].port_name, map->port_name ) == 0 )
			break;
	if( o == app->n_output_devices )
	{
		printf("N device %s not found\n", map->port_name);
		return 0;
	}
	write_buffer(
		app->output_devices[o].midi,
		data,
		n_data
	);
	printf("%s\n", app->output_devices[o].port_name);
	fflush(stdout);
	return 0;
}

void osc_setup_handler(const char *port_name, const char *ccc, const char *path, const char *vals)
{
	int assigns;
	struct osc_map *map = &osc_maps[num_maps++];
	map->port_name = port_name;
	assigns = sscanf( ccc, "%hhi,%hhi:%hhi", &map->ch, &map->cc_msb, &map->cc_lsb );
	map->ch -= 1;
	map->values[0] = 0xff;
	printf("V %s %d %d %d\n", map->port_name, map->ch, map->cc_msb, map->cc_lsb);

	int n = 0;
	char *copy = strdup(vals);
	char *pt;
	pt = strtok( copy, "," );
	while( pt != NULL )
	{
		assigns = sscanf( pt, "%hhi", &map->values[n++] );
		pt = strtok( NULL, "," );
	}
	free(copy);
	lo_server_thread_add_method(osc_server, path, "f", osc_cc_handler, map);
}

void osc_setup_handlers()
{
	unsigned int n = 0;
	FILE *fp = fopen("osc.conf","r");
	while( fscanf( fp, "%s\t%s\t%s\t%[^\n]", osc_config[n].port, osc_config[n].ccc, osc_config[n].path, osc_config[n].vals) != EOF)
	{
		osc_setup_handler(osc_config[n].port, osc_config[n].ccc, osc_config[n].path, osc_config[n].vals);
		n++;
	}
	fclose(fp);
}

void *osc_run()
{
	osc_server = lo_server_thread_new(OSC_PORT_STR, osc_error);
	lo_server_thread_add_method(osc_server, NULL, NULL, osc_default_handler, NULL);
	osc_setup_handlers();

	printf("N osc server listening on %s\n", OSC_PORT_STR);
	lo_server_thread_start(osc_server);

	while( ! stop_all )
	{
		sleep(1);
	}
	lo_server_thread_free(osc_server);
	printf("N osc server done\n");
	return 0;
}

pthread_t *setup_osc()
{
	pthread_create( &osc_thread, NULL, osc_run, NULL );
	return &osc_thread;
}
