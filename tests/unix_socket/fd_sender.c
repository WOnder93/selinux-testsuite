#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void usage(char *progname)
{
	fprintf(stderr,
		"usage:  %s socket-name file-to-send\n",
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
	char cmsgbuf[CMSG_SPACE(sizeof(payload_fd))];
	char databuf[1];

	if (argc != 3)
		usage(argv[0]);

	/* Part 0: Open the file */
	payload_fd = open(argv[2], O_RDWR);
	if (payload_fd < 0) {
		perror("open");
		exit(1);
	}

	/* Part 1: Set up the socket */
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	strcpy(sun.sun_path, argv[1]);
	sunlen = offsetof(struct sockaddr_un, sun_path) + strlen(sun.sun_path) + 1;

	if (connect(sock, &sun, sunlen) < 0) {
		perror("connect");
		close(sock);
		exit(1);
	}

	/* Part 2: Send the file handle */
	iov = (struct iovec) {
		.iov_base = databuf,
		.iov_len = 1
	};
	databuf[0] = 42;

	msg = (struct msghdr) {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cmsgbuf,
		.msg_controllen = sizeof(cmsgbuf),
	};
	cmsg = CMSG_FIRSTHDR(&msg);
	*cmsg = (struct cmsghdr) {
		.cmsg_level = SOL_SOCKET,
		.cmsg_type = SCM_RIGHTS,
		.cmsg_len = CMSG_LEN(sizeof(payload_fd)),
	};
	memcpy(CMSG_DATA(cmsg), &payload_fd, sizeof(payload_fd));

	if (sendmsg(sock, &msg, 0) < 0) {
		perror("sendmsg");
		close(sock);
		exit(1);
	}

	/* Cleanup */
	close(payload_fd);
	close(sock);
	return 0;
}
