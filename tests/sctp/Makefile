TARGETS = sctp_client sctp_server sctp_bind sctp_bindx sctp_connectx \
	sctp_asconf_params_client sctp_asconf_params_server sctp_peeloff_server

COMMON_SOURCES = sctp_common.c
COMMON_HEADERS = sctp_common.h
CFLAGS ?= -Wall

LDLIBS += -lselinux -lsctp

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

$(TARGETS): %: %.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $@.c $(COMMON_SOURCES) $(LDLIBS) $(LDFLAGS)
