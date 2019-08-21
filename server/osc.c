#include "common.h"
#include "osc.h"

pthread_t osc_thread;

void osc_error(int num, const char *msg, const char *path)
{
	printf("E osc liblo server error %d in path %s:\n", num, path, msg);
	fflush(stdout);
}

int osc_default_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
	int i;
	printf("N osc %s ", path);
	for (i = 0; i < argc; i++) {
		printf(" %d:%c:", i, types[i]);
		lo_arg_pp((lo_type)types[i], argv[i]);
		printf("\n");
	}
	printf("\n");
	fflush(stdout);
	return 1;
}

void *osc_run()
{
	lo_server_thread st = lo_server_thread_new(OSC_PORT_STR, osc_error);
	printf("N osc server listening on %s\n", OSC_PORT_STR);
	lo_server_thread_add_method(st, NULL, NULL, osc_default_handler, NULL);
	lo_server_thread_start(st);

	while( ! stop_all )
	{
		sleep(1);
	}
	lo_server_thread_free(st);
	printf("N osc server done\n");
	return 0;
}

pthread_t *setup_osc()
{
	pthread_create( &osc_thread, NULL, osc_run, NULL );
	return &osc_thread;
}
