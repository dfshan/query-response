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

#define MAX_RECV_SIZE 1048576

#include "main.h"
#include "sock.h"

int main(int argc, char **argv) {
	int clientfd;
	char *host, *port;
	char default_port[10];
	struct query_data query;
	int resp_size_kb;
	ssize_t recv_size, recv_left;
	char recv_buff[MAX_RECV_SIZE];
	memset(default_port, 0, sizeof(char) * 10);
	if (argc < 3) {
		fprintf(
			stderr,
			"USAGE: %s <response size in KB> <host> <port: optional, default %d>\n",
			argv[0], SERVER_PORT
		);
		return 0;
	}
	sprintf(default_port, "%d", SERVER_PORT);
	port = default_port;
	resp_size_kb = atoi(argv[1]);
	host = argv[2];
	if (argc > 3) {
		port = argv[3];
	}
	clientfd = open_clientfd(host, port);
	if (clientfd < 0) {
		perror("Open clientfd error");
		return 1;
	}
	recv_left = query.response_size_bytes = resp_size_kb * 1024;
	send_data(clientfd, sizeof(query), (char *) & query);
	while (recv_left > 0) {
		recv_size = recv(clientfd, recv_buff, MAX_RECV_SIZE-1, 0);
		if (recv_size < 0) {
			perror("recv()");
			break;
		} else if (recv_size == 0) {
			fprintf(stderr, "Socket peer has performed an orderly shutdown.\n");
			break;
		} else {
			recv_left -= recv_size;
		}
		if (recv_left == 0) {
			printf("Receive response of %dBytes(%dKB) data.\n", query.response_size_bytes, resp_size_kb);
		}
	}
	close(clientfd);
	return 0;
}
