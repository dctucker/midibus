#include <locale.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "app.h"

#define SHM_PATH "/midi-server"

struct app_t _app;
struct app_t *app = &_app;

void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	putc('\n', stderr);
}

void init_app()
{
	size_t bytes = sizeof(struct app_t);

	int fd;
	fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0644);
	if( fd < 0 )
	{
		error("Couldn't create shared memory descriptor: %s", strerror(errno));
		exit(-1);
	}
	ftruncate(fd, bytes);
	app = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if(app == MAP_FAILED)
	{
		error("Couldn't map shared memory object: %s", strerror(errno));
		exit(-1);
	}

	memset( app, 0, bytes );
	app->base = (intptr_t) app;
	stop_all = 0;

	setlocale(LC_NUMERIC, "");
	printf("M Hello. %'d bytes allocated to app at %d.\n", bytes, app->base);
}

void cleanup_app()
{
	shm_unlink(SHM_PATH);
}
