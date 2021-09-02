# Required for local building
INCLUDEDIR ?= /usr/include

TARGETS = check_binder client manager service_provider
LDLIBS += -lselinux -lrt
COMMON_SOURCES = binder_common.c
COMMON_HEADERS = binder_common.h

ifeq ($(shell test -e $(INCLUDEDIR)/linux/android/binderfs.h && echo true),true)
CFLAGS += -DHAVE_BINDERFS
TARGETS += check_binderfs
endif

ifneq (,$(findstring -DHAVE_BPF,$(CFLAGS)))
	COMMON_SOURCES += ../bpf/bpf_common.c
	COMMON_HEADERS += ../bpf/bpf_common.h
	LDLIBS += -lbpf
endif

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

$(TARGETS): %: %.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $@.c $(COMMON_SOURCES) $(LDLIBS) $(LDFLAGS)
