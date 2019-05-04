#include "filters.h"

FILTER_SETUP( ccmap )
{
	char* copy = strdup(args_name);
	unsigned char in_cc;
	char *pt;
	if( callback->args.ccmap.out_cc[127] == 0 )
		memset(callback->args.ccmap.out_cc, 0xff, 128);
	pt = strtok(copy, ","); callback->args.ccmap.channel = strtoul( pt, NULL, 10 ) - 1;
	pt = strtok(NULL, ","); in_cc = strtoul( pt, NULL, 16 );
	pt = strtok(NULL, ","); callback->args.ccmap.out_cc[in_cc] = strtoul( pt, NULL, 16 );
	printf("ccmap %d CC %d to %d",
		callback->args.ccmap.channel,
		in_cc,
		callback->args.ccmap.out_cc[in_cc]
	);
	free(copy);
}
FILTER_CALLBACK( ccmap )
{
	unsigned char cur_state;
	unsigned char exp_state = 0xb0 + args->ccmap.channel;
	unsigned char cc = 0xff, val = 0xff;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		unsigned char cur_state = scan_status(data, buf[b]);
		if( cur_state == exp_state && buf[b] < 0x80 )
		{
			if( cc == 0xff )
			{
				unsigned char out_cc = args->ccmap.out_cc[ buf[b] ];
				cc = out_cc < 0x80 ? out_cc : 0xfe;
			}
			else if( val == 0xff )
			{
				if( cc < 0x80 )
				{
					val = buf[b];
					out_buf[a++] = exp_state;
					out_buf[a++] = cc;
					out_buf[a++] = val;
				}
				val = 0xff;
				cc = 0xff;
			}
		}
		else
		{
			cc = 0xff;
			val = 0xff;
		}
	}
	return write_buffer( data->output_device->midi, out_buf, a );
}
