# Contributed binaries

## crawlpy-login.py

* Test the login prior crawling.
* Create wget-like login session cookie (useable by [sqlmap](http://sqlmap.org/))
* Dump login page (after login)

```shell
Usage: crawlpy-login.py -C conf.json [-c cookie.txt] [-o output.html] [-y] [-v]
       crawlpy-login.py -h
       crawlpy-login.py -V

crawlpy-login.py will test whether or not the specified crawlpy config
is valid and can successfully login.

You can optionally save a login session cookie (-c/--cookie) in wget format
which can be used by tools such as sqlmap.

You can also store the html output from a successfull/unsuccessful login
to file (-o/--output).


Required arguments:
  -C, --config=      Path to crawlpy json config.
                         -C /path/to/conf.json
                         --config=/path/to/conf.json

Optional arguments:
  -c, --cookie=      Path where to store the session cookie.
                         -c /path/to/cookie.txt
                         --cookie=/path/to/cookie.txt

  -o, --output=      Path where to store the html source after logging in.
                         -o /path/to/login.html
                         --cookie=/path/to/login.html

  -v, --verbose      Be more verbose.

  -y, --yes          Answer 'yes' to all questions.

System options:
  -h, --help         Show help.
  -V, --version      Show version information.
```