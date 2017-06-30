# HashPump

A tool to exploit the hash length extension attack in various hashing algorithms.

Currently supported algorithms: MD5, SHA1, SHA256, SHA512.

## Help Menu

```bash
$ hashpump -h
HashPump [-h help] [-t test] [-s signature] [-d data] [-a additional] [-k keylength]
    HashPump generates strings to exploit signatures vulnerable to the Hash Length Extension Attack.
    -h --help          Display this message.
    -t --test          Run tests to verify each algorithm is operating properly.
    -s --signature     The signature from known message.
    -d --data          The data from the known message.
    -a --additional    The information you would like to add to the known message.
    -k --keylength     The length in bytes of the key being used to sign the original message with.
    Version 1.2.0 with CRC32, MD5, SHA1, SHA256 and SHA512 support.
    <Developed by bwall(@botnet_hunter)>
```

## Sample Output

```bash
$ hashpump -s '6d5f807e23db210bc254a28be2d6759a0f5f5d99' --data 'count=10&lat=37.351&user_id=1&long=-119.827&waffle=eggo' -a '&waffle=liege' -k 14
0e41270260895979317fff3898ab85668953aaa2
count=10&lat=37.351&user_id=1&long=-119.827&waffle=eggo\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02(&waffle=liege
```

## Compile & install

```bash
$ git clone https://github.com/bwall/HashPump.git
$ apt-get install g++ libssl-dev
$ cd HashPump
$ make
$ make install
```

`apt-get` and `make install` require root privileges to run correctly.  The actual requirement is for `-lcrypto`, so depending on your operating system, your dependencies may vary.

On OS X HashPump can also be installed using [Homebrew](http://brew.sh/):

```bash
$ brew install hashpump
```

## Mentions

HashPump has been mentioned in a few write-ups.  If you are wondering how you can use HashPump, these are some great examples.

* http://ctfcrew.org/writeup/54
* http://d.hatena.ne.jp/kusano_k/20140310/1394471922 (JP)
* http://conceptofproof.wordpress.com/2014/04/13/plaidctf-2014-web-150-mtgox-writeup/
* http://achatz.me/plaid-ctf-mt-pox/
* http://herkules.oulu.fi/thesis/nbnfioulu-201401141005.pdf
* https://github.com/ctfs/write-ups/tree/master/plaid-ctf-2014/mtpox

## Python Bindings
Fellow Python lovers will be pleased with this addition.  Saving me from writing an implementation of all these hash algorithms with the ability to modify states in Python, Python bindings have been added in the form of hashpumpy.  This addition comes from [zachriggle](https://github.com/zachriggle).

### Installation
These Python bindings are available on [PyPI](https://pypi.python.org/pypi/hashpumpy/1.0) and can be installed via pip.
  pip install hashpumpy
  
### Usage
    >>> import hashpumpy
    >>> help(hashpumpy.hashpump)
    Help on built-in function hashpump in module hashpumpy:
    
    hashpump(...)
        hashpump(hexdigest, original_data, data_to_add, key_length) -> (digest, message)
    
        Arguments:
            hexdigest(str):      Hex-encoded result of hashing key + original_data.
            original_data(str):  Known data used to get the hash result hexdigest.
            data_to_add(str):    Data to append
            key_length(int):     Length of unknown data prepended to the hash
    
        Returns:
            A tuple containing the new hex digest and the new message.
    >>> hashpumpy.hashpump('ffffffff', 'original_data', 'data_to_add', len('KEYKEYKEY'))
    ('e3c4a05f', 'original_datadata_to_add')

### Python 3 note
hashpumpy supports Python 3. Different from the Python 2 version, the second value (the new message) in the returned tuple from `hashpumpy.hashpump` is a bytes-like object instead of a string.
