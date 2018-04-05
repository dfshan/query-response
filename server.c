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

struct thread_data {
	pthread_t tid;
	int connfd;
};

void *respond_to_client(void *targs);
int main(int argc, char **argv) {
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_in client_addr;
	int listen_port = SERVER_PORT;
	struct thread_data *tdata;
	if (argc > 1) {
		listen_port = atoi(argv[1]);
	}
	clientlen = sizeof(client_addr);
	listenfd = open_listenfd(listen_port);
	if (listenfd < 0) {
		perror("Open listenfd error");
		return 1;
	}
	while (1) {
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
        printf("connfd = %d\n", connfd);
		tdata = NULL;
		tdata = (struct thread_data *) malloc(sizeof(struct thread_data));
		tdata->connfd = connfd;
		pthread_create(&tdata->tid, NULL, respond_to_client, (void *)tdata);
	}
}

void *respond_to_client(void *targs) {
	struct thread_data *args = (struct thread_data *) targs;
	size_t query_size = sizeof(struct query_data);
	ssize_t recv_size;
	struct query_data query;
	char send_buff[MAX_SEND_SIZE];
	memset(send_buff, 0, sizeof(char) * MAX_SEND_SIZE);
	memset(&query, 0, sizeof(query));
	while (1) {
		recv_size = recv(args->connfd, (char *) &query, query_size, 0);
		if (recv_size < 0) {
			perror("recv()");
			break;
		} else if (recv_size == 0) {
			fprintf(stderr, "Socket peer has performed an orderly shutdown.\n");
			break;
		} else if (recv_size < (ssize_t) query_size) {
			fprintf(stderr, "Query size (%ldB) too small\n", recv_size);
			break;
		}
		printf(
			"Receive query for %dBytes (%dKB, %dMB) data.\n",
			query.response_size_bytes,
			query.response_size_bytes/1024,
			query.response_size_bytes/1024/1024
		);
		send_data(args->connfd, query.response_size_bytes, send_buff);
	}
	close(args->connfd);
	pthread_detach(pthread_self());
	free(args);
	return NULL;
}
