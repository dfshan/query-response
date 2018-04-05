#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_SEND_SIZE 1048576

#include "main.h"
#include "sock.h"

int main(int argc, char **argv) {
	int listenfd, connfd ;
	socklen_t clientlen;
	struct sockaddr_in client_addr;
	size_t query_size = sizeof(struct query_data);
	ssize_t recv_size;
	struct query_data query;
	char send_buff[MAX_SEND_SIZE];
	int listen_port = SERVER_PORT;
	if (argc > 1) {
		listen_port = atoi(argv[1]);
	}
	memset(send_buff, 0, sizeof(char) * MAX_SEND_SIZE);
	clientlen = sizeof(client_addr);
	listenfd = open_listenfd(listen_port);
	if (listenfd < 0) {
		perror("Open listenfd error");
		return 1;
	}
	while (1) {
		memset(&query, 0, sizeof(query));
		connfd = accept(listenfd, (struct sockaddr*) &client_addr, &clientlen);
		if (connfd < 0) {
			perror("aceept() error");
			continue;
		}
		printf(
			"Connection from host %s, port %d.\n",
			inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port)
		);
		recv_size = recv(connfd, (char *) &query, query_size, 0);
		if (recv_size < 0) {
			perror("recv()");
			close(connfd);
			continue;
		} else if (recv_size == 0) {
			fprintf(stderr, "Socket peer has performed an orderly shutdown.\n");
			close(connfd);
			continue;
		} else if (recv_size < (ssize_t) query_size) {
			fprintf(stderr, "Query size (%ld) too small\n", recv_size);
			close(connfd);
			continue;
		}
		printf(
			"Receive query for %dBytes (%dKB) data.\n",
			query.response_size_bytes,
			query.response_size_bytes/1024
		);
		send_data(connfd, query.response_size_bytes, send_buff);
		close(connfd);
	}
}
