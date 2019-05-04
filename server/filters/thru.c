#include "filters.h"

FILTER_SETUP( thru )
{
	printf("thru", args_name);
}
FILTER_CALLBACK( thru )
{
	for( int b = 0; b < n_bytes; ++b )
		scan_status(data, buf[b]);
	return write_buffer( data->output_device->midi, buf, n_bytes );
}

FILTER_SETUP( none )
{
	printf("none");
}
FILTER_CALLBACK( none )
{
	return 0;
}
