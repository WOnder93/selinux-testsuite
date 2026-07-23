#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

void usage(char *progname)
{
	fprintf(stderr,
		"usage:  %s socket-name\n",
		progname);
	exit(1);
}

int main(int argc, char **argv)
{
	int sock, payload_fd;
	struct sockaddr_un sun = { .sun_family = AF_UNIX };
	socklen_t sunlen;
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];
	char databuf[1];

	if (argc != 2)
		usage(argv[0]);

	/* Part 0: Set up the socket */
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	strcpy(sun.sun_path, argv[optind]);
	sunlen = offsetof(struct sockaddr_un, sun_path) + strlen(sun.sun_path) + 1;

	if (connect(sock, &sun, sunlen) < 0) {
		perror("connect");
		close(sock);
		exit(1);
	}

	/* Part 1: Receive a file from the forwarder */
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
	if (recvmsg(sock, &msg, 0) < 0) {
		perror("recvmsg");
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
	close(sock);

	if (payload_fd < 0) {
		fprintf(stderr, "error: no file received!\n");
		exit(1);
	}

	/* Part 2: Verify that we can read/write the file */
	if (write(payload_fd, databuf, sizeof(databuf)) < 0) {
		perror("write");
		close(payload_fd);
		exit(1);
	}

	if (read(payload_fd, databuf, sizeof(databuf)) < 0) {
		perror("read");
		close(payload_fd);
		exit(1);
	}

	/* Cleanup */
	close(payload_fd);
	return 0;
}
