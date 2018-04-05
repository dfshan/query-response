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

#define WARMUP_SIZE 1048576
#define MAX_RECV_SIZE 1048576
#define HOST_LEN 20
#define MAX_HOST 40

#include "main.h"
#include "sock.h"

struct query_item {
	char hostname[HOST_LEN];
	int clientfd;
	int resp_size_kb;
};

int query_data(int clientfd, int resp_size_kb) {
	ssize_t recv_size, recv_left;
	char recv_buff[MAX_RECV_SIZE];
	struct query_data query;
	query.response_size_bytes = resp_size_kb * 1024;
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
			printf(
				"Receive response of %dBytes(%dKB, %dMB) data.\n",
				query.response_size_bytes,
				resp_size_kb,
				resp_size_kb/1024
			);
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	int i, host_num;
	char *port;
	char default_port[10];
	struct query_item query[MAX_HOST];
	FILE *fp = NULL;
	fp = fopen("host.info", "r");
	if (fp == NULL) {
		perror("fopen error");
		return 1;
	}
	i = 0;
	while (1) {
		int ret;
		ret = fscanf(fp, "%s %d\n", query[i].hostname, &query[i].resp_size_kb);
		if (ret == EOF) {
			fclose(fp);
			break;
		} else if (ret <= 0) {
			fprintf(stderr, "fscanf error.\n");
			fclose(fp);
			break;
		}
		i += 1;
	}
	host_num = i;
	memset(default_port, 0, sizeof(char) * 10);
	sprintf(default_port, "%d", SERVER_PORT);
	port = default_port;
	i = 0;
	for (i = 0; i < host_num; i++) {
		query[i].clientfd = open_clientfd(query[i].hostname, port);
		if (query[i].clientfd < 0) {
			fprintf(stderr, "Open clientfd error (%s:%s): %s", query[i].hostname, port, strerror(errno));
			return 1;
		}
	}
	for (i = 0; i < host_num; i++) {
		query_data(query[i].clientfd, WARMUP_SIZE);
	}
	for (i = 0; i < host_num; i++) {
		printf(
			"query %s for %dKB(%dMB) data\n",
			query[i].hostname,
			query[i].resp_size_kb,
			query[i].resp_size_kb/1024
		);
		query_data(query[i].clientfd, query[i].resp_size_kb);
	}
	for (i = 0; i < host_num; i++) {
		close(query[i].clientfd);
	}
	return 0;
}

