TARGETS = bpf_test
COMMON_SOURCES = bpf_common.c
COMMON_HEADERS = bpf_common.h
LDLIBS += -lselinux -lbpf

# export so that BPF_ENABLED entries get built correctly on local build
export CFLAGS += -DHAVE_BPF

BPF_ENABLED = ../fdreceive ../binder

all: $(TARGETS)
	@set -e; for i in $(BPF_ENABLED); do $(MAKE) -C $$i all ; done

clean:
	rm -f $(TARGETS)
	@set -e; for i in $(BPF_ENABLED); do $(MAKE) -C $$i clean ; done

bpf_test: bpf_test.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o bpf_test bpf_test.c $(COMMON_SOURCES) $(LDLIBS) $(LDFLAGS)
