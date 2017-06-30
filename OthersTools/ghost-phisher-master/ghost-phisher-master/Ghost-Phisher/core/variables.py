# Metasploit Payloads for Windows
metasploit_windows_payloads = [ 'windows/meterpreter/reverse_tcp','windows/meterpreter/bind_tcp','windows/meterpreter/reverse_ipv6_tcp','windows/meterpreter/reverse_tcp_dns','windows/shell/reverse_tcp_allports', 'windows/vncinject/bind_nonx_tcp', 'windows/shell/bind_tcp', 'windows/vncinject/bind_ipv6_tcp', 'windows/patchupdllinject/reverse_tcp', 'windows/x64/meterpreter/bind_tcp', 'windows/dllinject/reverse_http', 'windows/patchupdllinject/find_tag', 'windows/upexec/reverse_ord_tcp', 'windows/patchupmeterpreter/bind_ipv6_tcp', 'windows/x64/shell/bind_tcp', 'windows/meterpreter/reverse_https', 'windows/x64/vncinject/bind_tcp', 'windows/meterpreter/reverse_nonx_tcp', 'windows/dllinject/reverse_tcp_dns', 'windows/meterpreter/reverse_http', 'windows/shell/reverse_ipv6_tcp', 'windows/vncinject/reverse_tcp_allports', 'windows/patchupmeterpreter/reverse_tcp', 'windows/patchupmeterpreter/find_tag', 'windows/patchupmeterpreter/bind_nonx_tcp', 'windows/dllinject/reverse_ord_tcp', 'windows/x64/meterpreter/reverse_tcp', 'windows/patchupdllinject/bind_tcp', 'windows/upexec/bind_ipv6_tcp', 'windows/vncinject/reverse_tcp', 'windows/meterpreter/reverse_tcp_allports', 'windows/patchupmeterpreter/reverse_nonx_tcp', 'windows/upexec/bind_tcp', 'windows/shell/bind_nonx_tcp', 'windows/dllinject/bind_ipv6_tcp', 'windows/patchupmeterpreter/reverse_ord_tcp', 'windows/meterpreter/bind_ipv6_tcp', 'windows/x64/shell_bind_tcp', 'windows/dllinject/reverse_ipv6_tcp', 'windows/dllinject/reverse_tcp', 'windows/x64/shell_reverse_tcp', 'windows/upexec/reverse_tcp', 'windows/upexec/find_tag', 'windows/patchupdllinject/reverse_ord_tcp', 'windows/patchupmeterpreter/reverse_ipv6_tcp', 'windows/upexec/reverse_tcp_dns', 'windows/patchupdllinject/bind_nonx_tcp', 'windows/patchupmeterpreter/reverse_tcp_allports', 'windows/dllinject/bind_nonx_tcp', 'windows/x64/vncinject/reverse_tcp', 'windows/vncinject/reverse_http', 'windows/shell/reverse_tcp_dns', 'windows/dllinject/reverse_nonx_tcp', 'windows/patchupmeterpreter/bind_tcp', 'windows/dllinject/find_tag', 'windows/shell/bind_ipv6_tcp', 'windows/dllinject/reverse_tcp_allports', 'windows/patchupdllinject/reverse_nonx_tcp', 'windows/vncinject/bind_tcp', 'windows/x64/exec', 'windows/dllinject/bind_tcp', 'windows/x64/shell/reverse_tcp', 'windows/vncinject/reverse_ord_tcp', 'windows/patchupdllinject/reverse_tcp_dns', 'windows/patchupdllinject/bind_ipv6_tcp', 'windows/meterpreter/find_tag', 'windows/vncinject/find_tag', 'windows/meterpreter/reverse_ord_tcp', 'windows/patchupdllinject/reverse_ipv6_tcp', 'windows/vncinject/reverse_ipv6_tcp', 'windows/shell/reverse_http', 'windows/meterpreter/bind_nonx_tcp', 'windows/vncinject/reverse_tcp_dns', 'windows/patchupmeterpreter/reverse_tcp_dns', 'windows/shell/find_tag', 'windows/shell/reverse_tcp', 'windows/upexec/bind_nonx_tcp', 'windows/shell/reverse_nonx_tcp', 'windows/shell/reverse_ord_tcp', 'windows/upexec/reverse_nonx_tcp', 'windows/upexec/reverse_tcp_allports', 'windows/upexec/reverse_ipv6_tcp', 'windows/patchupdllinject/reverse_tcp_allports', 'windows/upexec/reverse_http', 'windows/vncinject/reverse_nonx_tcp']

# Metasploit Payloads for Linux
metasploit_linux_payloads = ['linux/x86/shell/reverse_tcp', 'linux/x86/shell_reverse_tcp','linux/x86/adduser','linux/x86/shell_reverse_tcp2','linux/x86/shell_bind_ipv6_tcp','linux/x86/shell_find_tag', 'linux/x86/shell_find_port', 'linux/x86/shell_bind_tcp','linux/x86/shell/reverse_ipv6_tcp', 'linux/x86/shell/find_tag', 'linux/x86/shell/bind_tcp', 'linux/x86/shell/bind_ipv6_tcp', 'linux/x86/metsvc_reverse_tcp', 'linux/x86/metsvc_bind_tcp', 'linux/x86/meterpreter/reverse_tcp', 'linux/x86/meterpreter/reverse_ipv6_tcp', 'linux/x86/meterpreter/find_tag', 'linux/x86/meterpreter/bind_tcp', 'linux/x86/meterpreter/bind_ipv6_tcp', 'linux/x86/exec', 'linux/x86/chmod', 'linux/x64/shell_reverse_tcp', 'linux/x64/shell_bind_tcp', 'linux/x64/shell/reverse_tcp', 'linux/x64/shell/bind_tcp', 'linux/x64/exec', 'linux/ppc64/shell_reverse_tcp', 'linux/ppc64/shell_find_port', 'linux/ppc64/shell_bind_tcp', 'linux/ppc/shell_reverse_tcp', 'linux/ppc/shell_find_port', 'linux/ppc/shell_bind_tcp', 'linux/mipsle/shell_reverse_tcp', 'linux/mipsbe/shell_reverse_tcp', 'linux/armle/shell_reverse_tcp', 'linux/armle/exec', 'linux/armle/adduser']

# Metasploit Encoders
metasploit_encoders = ['x86/shikata_ga_nai','cmd/generic_sh', 'cmd/ifs', 'cmd/printf_php_mq', 'generic/none', 'mipsbe/longxor', 'mipsle/longxor', 'php/base64', 'ppc/longxor', 'ppc/longxor_tag', 'sparc/longxor_tag', 'x64/xor', 'x86/alpha_mixed', 'x86/alpha_upper', 'x86/avoid_utf8_tolower', 'x86/call4_dword_xor', 'x86/context_cpuid', 'x86/context_stat', 'x86/context_time', 'x86/countdown', 'x86/fnstenv_mov', 'x86/jmp_call_additive', 'x86/nonalpha', 'x86/nonupper', 'x86/single_static_bit', 'x86/unicode_mixed', 'x86/unicode_upper']



# GHOST TRAP Default Vulnerability page variables

# variables here are:
# "ghost_date" = date
# 'ghost_file_size' = filesize
# 'ghost_payload_executable' = executable path
