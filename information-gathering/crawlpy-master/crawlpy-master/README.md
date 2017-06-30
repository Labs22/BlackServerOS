# Crawlpy

---

Python web spider/crawler based on [scrapy](http://scrapy.org/) with support for POST/GET login, variable level of recursions/depth and optionally save to disk.

---


## Requirements

* [python 2.7](https://www.python.org/)
* [lxml](http://lxml.de/)
* [pip](https://pip.pypa.io/en/latest/installing/)

```shell
pip install Scrapy
```


## Features

* POST/GET Login prior crawling
* Can handle logins that requires dynamic CSRF token
* Variable level of crawling depth
* Optionally save webpages to disk

## Roadmap

Find all planned features and their stati here: https://github.com/cytopia/crawlpy/issues/1


## Usage

```bash
# stdout output
scrapy crawl crawlpy -a config=/path/to/crawlpy.config.json

# save as json (url:, status:, depth:, referer:) to 'urls.json'
scrapy crawl crawlpy --loglevel=INFO -a config=/path/to/crawlpy.config.json -o urls.json -t json

# save as csv (url, status, depth, referer) to 'urls.csv'
scrapy crawl crawlpy --loglevel=INFO -a config=/path/to/crawlpy.config.json -o urls.csv -t csv
```

## Configuration

Make a copy of [crawlpy.config.json-sample](crawlpy.config.json-sample) (e.g.: `example.com-config.json`) and adjust the values accordingly.

**Note:**
It must be a valid json file (without comments), otherwise `crawlpy` will throw errors parsing json. (Use http://jsonlint.com/ to validate your config file.)

```json
{
    "proto": "http",        // 'http' or 'https'
    "domain": "localhost",  // Only the domain. e.g.: 'example.com' or 'www.example.com'
    "depth": 3,             // Nesting depth to crawl
    "ignores": [],          // Array of substrings to deny/ignore when found in URL
	"httpstatus_list": [],  // Array of http status codes to handle (default is 2xx)
    "login": {              // Login section
        "enabled": false,   // Do we actually need to do a login?
        "method": "post",   // 'post' or 'get'
        "action": "/login.php", // Where the post or get will be submitted to
        "failure": "Password is incorrect", // The string you will see on login failure
        "fields": {         // POST/GET Fields to submit to login page
            "username": "john",
            "password": "doe"
        },
        "csrf": {
            "enabled": false, // Login requires a CSRF token?
            "field": "csrf"   // Input field name that holds dynamic CSRF token
        }
    },
    "store": {              // Store section
        "enabled": false,   // save to disk?
        "path": "./data"    // path for saving (rel or abs)
    }
}
```

### Detailed description

|Key|Type|Default Value|Possible Values|Description|
|---|----|-------------|---------------|-----------|
|proto|string|`http`|`http` or `https`|Is the site you want to crawl running on `http` or `https`?|
|domain|string|`localhost`|Domain or subdomain|The domain or subdomain you want to spider. Nothing outside this domain/subdomain will be touched.|
|depth|integer|`3`|`0`,`1`,`2`,`3`,...|`0`: Crawl indefinetely until every subpage has been reached.<br/>`1`: Only crawl links on the initial page.<br/>`2`: Crawl links on the initial page and everything found on the links of that page.<br/><br/>**Note:** when you do a login, the login page already counts as one level of depth by scrapy itself, but this is rewritten internally to subtract that depth again, so your output will not show that extra depth.|
|ignores|array|\[ \]|\['/logout.php', 'delete.php?id=1'\]|Each array string element is treated as a substring (no regex) and is checked against a FQDN. If any of the specified substrings is found in that URL, it will not be crawled.<br/><br/>**Note:** It does make sense, when you login somewhere, to ignore the logout page, as well as other pages that might delete/disable your current user, so you will not be kicked from your login session during crawl time.|
|httpstatus_list|array|\[ \]|\[403, 404, 500\]|By default scrapy ignores pages with status code other than 2xx, so if you know that a 403 page contains actual content with links, just add this here.<br/><br/>**Note:** There is no need to specify `200`, as scrapy crawls them by default.|
|**login**||||Login section|
|enabled|boolean|`false`|`true` or `false`|`true`: Do a login prior crawling<br/>`false`: do not login<br/><br/>**Note:**When login is set to `false`, you do not need to fill in the rest of the variables inside the `login` section|
|method|string|`post`|`post` or `get`|Method required to execute the login|
|action|string|`/login.php`|login page|Relative login page (from the base domain, including leading slash) where the `post` or `get` will go to.|
|failure|string|`Password is incorrect`|login failed string|A string that is found on the login page, when the login fails.|
|fields|key-value|`{`<br/>    `"username": "john",`<br/>    `"password": "doe"`<br>`}`|`post` or `get` params|POST or GET params required to login.<br/><br/>**Examples:** username, password, hidden-field-name|
|**csrf**||||Login CSRF section|
|enabled|boolean|`false`|`true` or `false`|`true`: Login page has a dynamic CSRF token that you want to read out and submit along the normal submit data.<br/>`false`: Login does not require a CSRF token to be submitted.<br/><br/>**Note:** If the login has a static (never-changing) CSRF field, just add the data into the fields section<br/>**Note:** Read below about built-in automatic CSRF detection and leave this off at first.|
|field|string|`csrf`|Field name|The name of the input field which holds the CSRF token|
|**store**||||Store section|
|enabled|boolean|`false`|`true` or `false`|`true`: Save webpages to disk<br/>`false`: Do not save webpages to disk.|
|path|string|`./data`|Path|Absolute or relative path to store webpages to disk|


### Note about CSRF

Scrapy will most likely handle this automatically, so its best to turn off custom `csrf` in the config. If there is however any situation where the built-in CSRF recognition does not work, try the user-defined one. If none of them work, drop me an issue.


## Reference

* https://stackoverflow.com/questions/5851213/crawling-with-an-authenticated-session-in-scrapy
* https://stackoverflow.com/questions/38619807/scrapy-recursive-link-crawler-with-login-help-me-improve/
* http://thuongnh.com/building-a-web-crawler-with-scrapy/
