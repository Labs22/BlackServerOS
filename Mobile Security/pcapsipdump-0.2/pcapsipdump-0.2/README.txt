pcapsipdump version 0.2

Usage: pcapsipdump [-fpU] [-i <interface>] [-r <file>] [-d <working directory>] [-v level] [-R filter]
 -f   Do not fork or detach from controlling terminal.
 -p   Do not put the interface into promiscuous mode.
 -R   RTP filter. Possible values: 'rtp+rtcp' (default), 'rtp', 'rtpevent', 't38', or 'none'.
 -U   Make .pcap files writing 'packet-buffered' - slower method,
      but you can use partitially written file anytime, it will be consistent.
 -v   Set verbosity level (higher is more verbose).
 -n   Number-filter. Only calls to/from specified number will be recorded
      (to use regular expressions here, compile with: 'make DEFS=-DUSE_REGEXP')
 -t   T.38-filter. Only calls, containing T.38 payload indicated in SDP will be recorded

pcapsipdump is a tool for dumping SIP sessions (+RTP
traffic, if available) to disk in a fashion similar
to "tcpdump -w" (format is exactly the same), but one
file per sip session (even if there is thousands of
concurrent SIP sessions).

pcapsipdump can also be used to split "bulk" pcap file
into bunch of individual files (one per call):
pcapsipdump -r <bulkfile> -d <dir-for-bunch-of-files>

for Red Hat/CentOS/Fedora rpm instructions see redhat/ dir
for Debian-specific instructions, see debian/ dir