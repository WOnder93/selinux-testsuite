TARGETS=client server socketpair fd_forwarder fd_receiver fd_sender

LDLIBS+= -lselinux

all: $(TARGETS)
clean:
	rm -f $(TARGETS) flag test_client_sock test_server_sock
