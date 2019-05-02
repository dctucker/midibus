#pragma once

#include <pthread.h>

#define UDP_PORT 13949
#define PORT 13949

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

//char *socket_path = "./socket";
//char *socket_path = "../midi-server.sock";
#define MAX_CLIENTS 16

pthread_t *setup_socket();
void emit_config();
void emit_devices();
void *socket_thread();
