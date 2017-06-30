# ophcrack (Time-Memory-Trade-Off-Crack) #


## About ##

A windows password cracker based on the faster time-memory trade-off using
rainbow tables.

This is an evolution of the original ophcrack 1.0 developed at EPFL 
(http://lasecwww.epfl.ch/~oechslin/projects/ophcrack)

Ophrack comes with a Qt Graphical User Interface which runs on Windows, 
Mac OS X as well as on Unix. 


## Install ophcrack ##

Ophcrack can be downloaded from sourceforge: http://ophcrack.sourceforge.net

Binaries compiled for Windows are provided. These binaries are standalone
(portable) and no installation is required. 

The Linux version is a source package. It can be compiled and 
installed using these commands:
```
./configure
make
make install
```


### Tables ###

The tables have to be downloaded manually:
http://ophcrack.sourceforge.net/tables.php



## HOWTO ##

This howto assumes you have already installed ophcrack 3 and downloaded the
ophcrack rainbow tables you want to use. It also assumes that you understand how
to use third party tools like pwdump or mimikatz
(https://github.com/gentilkiwi/mimikatz) to dump the SAM of a Windows system.

Ophcrack and the ophcrack LiveCD are available for free at the ophcrack project
page (http://ophcrack.sourceforge.net/).

Ophcrack rainbow tables are available at ophcrack rainbow tables page
(http://ophcrack.sourceforge.net/tables.php).


### First step (optional) ###

This step is optional but will speed up the cracking process.

Run ophcrack and set the number of threads under the Preferences tab to the
number of cores of the computer running ophcrack.


### Second step ###

Load hashes using the Load button. You can either enter the hash manually
(Single hash option), import a text file containing hashes you created with
pwdump, mimikatz or similar third party tools (PWDUMP file option), extract the
hashes from the SYSTEM and SAM files (Encrypted SAM option) or dump the SAM from
the computer ophcrack is running on (Local SAM option).

For the Encrypted SAM option, the SAM is located under the Windows
system32/config directory and can only be accessed for a Windows partition that
is NOT running. For the Local SAM option, you MUST execute ophcrack with the
administrator rights on the computer you want to dump the SAM.


### Third step (optional) ###

This step is optional but will speed up the cracking process.

Delete with the Delete button every user account you are not interested in (for
exemple the Guest account). You can use the Ctrl key to make multiple selection.
Ctrl-a will select every loaded hash.

Keep in mind that the time needed to crack password hashes with rainbow tables
is proportional to the number of hashes loaded. With a brute force attack the
cracking time is NOT dependant on the number of unsalted hashes loaded. That's
why it's advisable to remove any unnecessary user account with the Delete
button.


### Fourth step ###

Install (Tables button), enable (green and yellow buttons) and sort wisely (up
and down arrows) the rainbow tables your are going to use. Keep in mind that
storing the rainbow tables on a fast medium like a SSD will significantly
speed up the cracking process.


### Fifth step ###

Click on the Crack button to start the cracking process. You'll see the progress
of the cracking process in the bottom boxes of the ophcrack window. When a
password is found, it will be displayed in the NT Pwd field. You can then save
the results of a cracking session at any time with the Save button.
