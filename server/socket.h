#pragma once
#define UDP_PORT 13949

int sockfd;

void setup_socket();
void send_config();
void send_devices();
void *socket_thread();
