#pragma once
#define UDP_PORT 13949

int sockfd;

void setup_socket();
void *socket_thread();
