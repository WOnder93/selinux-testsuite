TARGETS = client server

ifneq (,$(findstring -DHAVE_BPF,$(CFLAGS)))
	COMMON_SOURCES = ../bpf/bpf_common.c
	COMMON_HEADERS = ../bpf/bpf_common.h
	LDLIBS += -lbpf
endif

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

client: client.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o client client.c $(COMMON_SOURCES) $(LDLIBS) $(LDFLAGS)
