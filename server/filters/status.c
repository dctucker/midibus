#include "filters.h"

FILTER_SETUP( status )
{
	char* copy = strdup(args_name);
	char *pt;
	printf("status", args_name);

	pt = strtok(copy, ",");
	while (pt != NULL)
	{
		unsigned char a = strtoul( pt, NULL, 16 );
		callback->args.status.out_status[ a - 0x80 ] = a;
		printf(" 0x%X", a);
		pt = strtok(NULL, ",");
	}
	free(copy);
}
FILTER_CALLBACK( status )
{
	unsigned char *out_status = args->status.out_status;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		unsigned char cur_state = scan_status(data, buf[b]);
		if( out_status[ cur_state - 0x80 ] == cur_state )
		{
			out_buf[a++] = buf[b];
		}
	}
	return write_buffer( data->output_device->midi, out_buf, a );
}

