#ifndef SOCK_H
#define SOCK_H

#define LISTENQ 1024

int open_clientfd(char *hostname, char *port);
int open_listenfd(int port);
int send_data(int sockfd, size_t send_size, char *buff);

#endif
