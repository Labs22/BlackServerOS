hashfind
========

Hashfind - A tool to search files for matching password hash types and other interesting data.

Hashfind allows you to search single files, multiple files or entire directories and subdirectories for matches to 
common hash types. Supported password hash types that are extracted include;

Raw MD5
MySQL (old)
Joomla
vBulletin
phpbb3
WordpressMD5
Drupal
Unix MD5 (old)
SHA512 Crypt
Emails
Credit-Card Numbers

More will be added later.

The program can either write to a specified output file or directly to stdout in quiet mode.

NOTE
----
The program may not detect a hash if it was broken up over multiple lines and split with a \n .
This will be resolved in a future version, however as yet demand for this has not been too great.

