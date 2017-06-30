Doozer
========
Doozer is an extensible, automated hash cracking utility used to automate the parsing, cracking, and organization of password hashes.  Currently, Doozer supports nt/lm, ntlmv1, and ntlmv2 hashes (from Responder or SAM), but sports a modular architecture that allows other hash types to be trivially plugged in.  A simple, Django-based web interface allows users to view cracking sessions in real time, view session results, and search the hash database.  A simple REST API is also exposed, allowing scripts to easily interface with the database.

Doozer features a folder monitoring solution; in order to provide a seamless method for cracking hashes, Doozer allows you to set a folder that it will monitor for hash files.  Once a file is dropped into this location, Doozer will move it to a working directory and queue it for cracking.  Doozer supports a configurable number of parallel cracking sessions as well as a simple queuing system to automatically start up sessions once one has completed.  This is configured in `doozer/core/sethor.py`.

There are bugs, and Doozer is currently still in development.

Hash view:
![Hashes](http://i.imgur.com/XG0RWDl.jpg)

Session view:
![Session](http://i.imgur.com/UYqvxFs.jpg)

Doozer requires
========
* django-1.5+ (tested on 1.5.4)
* django-bootstrap3
* watchdog
* requests
* passlib

Doozer contains
===========
* monitor.py
  -- Manages monitoring our watched folder, running, and spinning up jobs.

* horstop.py
  -- Auxiliary tool for interacting with masterhor.  Can be used to insert 
  data into the master list.

* doozer.py 
  -- doozer - the next generation

To run: 

    $ ./doozer/horstop.py --startup 

This kicks off our Django application as well as our monitoring application

USAGE
====

Please see the wikipedia entry for usage information
    
TODO
====
* roll over monitor_log and create a log archive
* limit default hash page to top 100
* add more awesome hash types
* provide more hash statistics (reports?)
