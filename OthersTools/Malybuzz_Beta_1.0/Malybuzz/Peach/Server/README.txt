
Peach Test Server
=================

This folder contains a simple client/server addition to peach that
allows for recording test runs.  This is especially usefull when fuzzing
something that may topple before you can determin which test caused 
the failure (like anything in kernel).

Dependencies
------------

 - MySQL & MySQL Python module (server only)
 - Twisted python module (server only)

Usage
-----

1. Add client code to your fuzzer

2. Start up the server:

   python twistedserver.py PORT DB_HOST DB_NAME DB_USER DB_PASSWD

3. Run your fuzzer!


Tips
----

Always use an IP addresss in the URL given to PeachClient.  For some reason
dns lookup is ass-slow and will add 1-2 seconds onto each test.


The Fuzzer Side
---------------

Import the client.py file and create a PeachClient object.  For example:

peachClient = PeachClient('http://127.0.0.1:9001')
peachClient.RunStarting()

while True:
  peachClient.StartTest('test name', TEST_NUM, TEST_DATA)

  # Run test

  peachClient.EndTest('test name', TEST_NUM, TEST_RESULT)

peachClient.RunFinished()


# end
