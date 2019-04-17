#pragma once
#define UDP_PORT 13949

#include <pthread.h>

int sockfd;

pthread_t *setup_socket();
void emit_config();
void emit_devices();
void *socket_thread();
