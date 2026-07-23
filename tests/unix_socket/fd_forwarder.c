#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifndef MSG_CMSG_SEALED
#define MSG_CMSG_SEALED 0x10000000
#endif

void usage(char *progname)
{
	fprintf(stderr,
		"usage:  %s [-f file] socket-name\n",
		progname);
	exit(1);
}

int main(int argc, char **argv)
{
	int opt, sock, newsock, payload_fd;
	char *flag_file = NULL;
	struct sockaddr_un sun = { .sun_family = AF_UNIX };
	socklen_t sunlen;
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];
	char databuf[1];

	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
		case 'f':
			flag_file = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if ((argc - optind) != 1)
		usage(argv[0]);

	/* Part 0: Set up the socket */
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	strcpy(sun.sun_path, argv[optind]);
	sunlen = offsetof(struct sockaddr_un, sun_path) + strlen(sun.sun_path) + 1;

	if (bind(sock, (struct sockaddr *)&sun, sunlen) < 0) {
		perror("bind");
		close(sock);
		exit(1);
	}

	if (listen(sock, SOMAXCONN)) {
		perror("listen");
		close(sock);
		exit(1);
	}

	if (flag_file) {
		FILE *f = fopen(flag_file, "w");
		if (!f) {
			perror("Flag file open");
			close(sock);
			exit(1);
		}
		fprintf(f, "listening\n");
		fclose(f);
	}

	/* Part 1: Receive a file from the sender */
	newsock = accept(sock, NULL, NULL);
	if (newsock < 0) {
		perror("accept");
		close(sock);
		exit(1);
	}

	iov = (struct iovec) {
		.iov_base = databuf,
		.iov_len = 1
	};
	msg = (struct msghdr) {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cmsgbuf,
		.msg_controllen = sizeof(cmsgbuf),
	};
	if (recvmsg(newsock, &msg, MSG_CMSG_SEALED) < 0) {
		perror("recvmsg");
		close(newsock);
		close(sock);
		exit(1);
	}

	payload_fd = -1;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET &&
		    cmsg->cmsg_type == SCM_RIGHTS) {
			if (cmsg->cmsg_len - CMSG_LEN(0) != sizeof(int))
				continue;
			payload_fd = *(int *)CMSG_DATA(cmsg);
		}
	}
	close(newsock);
	if (payload_fd < 0) {
		fprintf(stderr, "error: no file received!\n");
		close(sock);
		exit(1);
	}

	/* Part 2: Verify that we CAN'T read/write the file */
	if (write(payload_fd, databuf, sizeof(databuf)) >= 0) {
		fprintf(stderr, "FAIL: write to payload file succeeded!\n");
		close(payload_fd);
		close(sock);
		exit(1);
	}

	if (read(payload_fd, databuf, sizeof(databuf)) >= 0) {
		fprintf(stderr, "FAIL: read from payload file succeeded!\n");
		close(payload_fd);
		close(sock);
		exit(1);
	}

	/* Part 3: Send the file to the receiver */
	newsock = accept(sock, NULL, NULL);
	if (newsock < 0) {
		perror("accept");
		close(sock);
		exit(1);
	}

	/* Just re-send the whole msg unchanged */
	if (sendmsg(newsock, &msg, MSG_CMSG_SEALED) < 0) {
		perror("sendmsg");
		close(newsock);
		close(sock);
		exit(1);
	}

	/* Cleanup */
	close(newsock);
	close(sock);
	return 0;
}
