# hunter
(l)user hunter using WinAPI calls only

### Introduction:

During Red Team engagements it is common to track/hunt specific users. Assuming we already have access to a desktop as a normal user (no matter how, always "assume compromise") __in a Windows Domain__ and we want to spread laterally. We want to know where the user is logged on, if he is a local administrator in any box, to which groups he belongs, if he has access to file shares, and so on. Enumerating hosts, users, and groups will also help to get a better understanding of the Domain layout.

You might be thinking, "use Powerview". Lately, one of the most common problems I encounter during Red Team exercises is the fact that __PowerShell is heavily monitored__. If you use it, you'll get caught, sooner or later. By now everyone is well aware how powerful PowerShell is, including Blue Teams and Security Vendors.

There are multiple ways to work around this. To avoid using multiple old school tools ([psloggedon.exe](https://technet.microsoft.com/en-us/sysinternals/psloggedon.aspx), [netsess.exe](http://www.joeware.net/freetools/), [nltest](https://technet.microsoft.com/en-us/library/cc731935%28v=ws.11%29.aspx), [netview](https://github.com/mubix/netview), among others) and to reduce the amount of tools uploaded to compromised systems I created a simple tool that __doesn't require Administrative privileges__ to run and collect the information listed below, and __relies only on the Windows API__.

You might end up dealing with white list bypass and process evasion, but I'll leave that for another day.

### What is it:

The __(l)user hunter__ tool is a small program written in C/C++ that uses WinAPI calls only to:

* Retrieves current configuration information for the specified server (via list of hosts or domain enumeration).
  - OS Version
  - Server Type (DC, Backup DC, Workstation or Server, Terminal Server, MSSQL Server)
* Lists information about all users currently logged on to the workstation.
  - interactive, service and batch logons.
* Lists information about sessions established on a server.
* Retrieves information about each shared resource on a server.
  - checks if current user as read access.
* Returns results for the NS_DNS namespace, IPv4 protocol.
* Checks if current user is an Administrator on a server.
* Retrieves information about all user accounts on a server or DC.
* Retrieves a list of global groups to which a specified user belongs on a server or DC.
* Retrieves information about each global group in the security database, SAM database or Active Directory.
* Retrieves a list of the members in a particular global group in the security database, SAM database or Active Directory.
* Retrieves information about a particular user account on a server or DC.
* Enumerate the domain controllers in the local domain.


Additionally, for hosts enumeration there's a minimum and maximum delay value in seconds you can add to avoid detection/noise.

### How to Compile it:

Grab a copy of Visual Studio, it's free. It won't build on Linux, maybe later.

### How to use it:

See below some usage examples.

![Alt text](screenshots/help.png?raw=true "Help")

![Alt text](screenshots/file.png?raw=true "Read hosts from file")

![Alt text](screenshots/domain.png?raw=true "Current domain")

![Alt text](screenshots/delay.png?raw=true "with Delay options")

![Alt text](screenshots/users.png?raw=true "Users")

![Alt text](screenshots/groups.png?raw=true "Groups")

![Alt text](screenshots/users_per_group.png?raw=true "Users per Group")

![Alt text](screenshots/groups_per_user.png?raw=true "Groups per User")

![Alt text](screenshots/user_info_host.png?raw=true "User info on a server")

![Alt text](screenshots/user_info.png?raw=true "User info")

![Alt text](screenshots/users_detailed.png?raw=true "Users information")

![Alt text](screenshots/dcs.png?raw=true "Domain Controllers")

### TODO:

* Improve Error handling
* Improve output
* Identify IPv6 interfaces
* Improve arg parsing
* Create command line flags to specify what queries to run
* Rewrite the random delay counter between queries

### Issues:

This is a beta version, use at your own risk.

* When Windows 10 is identified via network discovery some queries fail. If you query it via an IP address it works.
* Error handling is far from awesome.

### Credits:

Credits where they are due. MSDN is awesome and I grabbed a big part of the code from the examples provided. Just look for the API calls used for more informaton.

