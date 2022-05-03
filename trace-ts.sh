cd /root/testsuite

echo "p:trace_func selinux_sctp_bind_connect" >/sys/kernel/tracing/kprobe_events
perf record -o perf.data -a -g --call-graph dwarf -e kprobes:trace_func -- make test
perf script -i perf.data
