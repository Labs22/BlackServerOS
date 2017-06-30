Summary: PCAP SIP Dump tool
Name: pcapsipdump
Distribution: RedHat
Version: 0.2
Release: 1
License: GPL v2
Group: Utilities/System
Vendor: pcapsipdump.sf.net
BuildRoot: /var/tmp/%{name}-%{version}
Source: %{name}-%{version}.tar.gz

%description
pcapsipdump is a tool for dumping SIP sessions (+RTP
traffic, if available) to disk in a fashion similar
to "tcpdump -w" (format is exactly the same), but one
file per sip session (even if there is thousands of
concurrent SIP sessions).

%prep
%setup

%build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/sbin $RPM_BUILD_ROOT/etc/sysconfig $RPM_BUILD_ROOT/etc/rc.d/init.d $RPM_BUILD_ROOT/var/spool
make DESTDIR=$RPM_BUILD_ROOT install

%post
chkconfig pcapsipdump --add

%files
%config(noreplace) %attr(0755,root,root) /etc/sysconfig/pcapsipdump
%attr(0700,root,root) %dir    /var/spool/pcapsipdump
%attr(0755,root,root)       /etc/rc.d/init.d/pcapsipdump
%attr(0755,root,root)      /usr/sbin/pcapsipdump
