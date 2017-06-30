# Client

## Global

### crypton.host and crypton.port

Defaults to `localhost:2013`. You should override these before any other calls are made. Note that cross-domain requests may be enabled in the server configuration.

### crypton.on(eventName, callback)

### crypton.generateAcount(username, passphrase, callback)

_crypto_  

    algorithms:
        AES256 CFB and 128 bit segment width (16 bytes)
        RSA
        KDF (PBKDF2 w/ 50,000 rounds)

    generate: 
        32 random bytes hmac key for container names ("container_name_hmac_key")
        32 random bytes hmac key for general data authentication ("hmac_key")
        32 random bytes for ("salt_key")
        32 random bytes for ("salt_challenge")
        32 random bytes for ("symkey")
        rsa keypair (2048 bits) (rsa_keypair_obj)

    outputs:
        challenge_key = kdf(salt_challenge, passphrase) 
        keypair_key = kdf(salt_key, passphrase) 
        keypair_as_string = rsa_keypair_obj.serialize_to_string()
        pubkey_as_string = rsa_keypair_obj.public_key.serialize_to_string()
        keypair_iv = sha256(uuid()).digest()[:16]
        cipher = aes256cfb(key=keypair_key, iv=keypair_iv)
        # XXX padding (must be length of multiple of 16)
        keypair_serialized_ciphertext = cipher.encrypt(keypair_as_string) 
        symkey_ciphertext = rsa_keypair_obj.encrypt_to_private(symkey)
        container_name_hmac_key_iv = sha256(uuid()).digest()[:16]
        cipher = aes256cfb(key=symkey, iv=container_name_hmac_key_iv)
        container_name_hmac_key_ciphertext = cipher.encrypt(container_name_hmac_key)
        hmac_key_iv = sha256(uuid()).digest()[:16]
        cipher = aes256cfb(key=symkey, iv=hmac_key_iv)
        hmac_key_ciphertext = cipher.encrypt(hmac_key)


    steps:
        save this object:
            {
                hmac_key: 32 byte string, base64ed
                salt_key: 32 byte string, base64ed
                salt_challenge: 32 byte string, base64ed
                keypair_iv: 16 byte string, base64ed
                keypair_serialized_ciphertext: many bytes, base64ed
                pubkey_serialized: many bytes, base64ed
                challenge_key: 32 byte string, base64ed
                symkey_ciphertext: many bytes, base64ed
                container_name_hmac_key_iv: 16 byte string, base64ed
                container_name_hmac_key_ciphertext: 32 byte string, base64ed
                hmac_key_iv: 16 byte string, base64ed
                hmac_key_ciphertext: 32 byte string, base64ed
            }

    discard:
        keypair_key
        rsa_keypair_obj
        cipher objects
        symkey
        container_name_hmac_key
        hmac_key
        keypair_as_string
        pubkey_as_string

Creates an account object and generates the appropriate salts and keys. 

Checks with the server to validate the username, and calls back with a potentially empty `error` argument and a conditional `account` argument which must still be `save()`d.

### crypton.authorize(username, passphrase, callback)

_crypto_

 Step 1: get a challenge from the server.

    The server constructs a challenge like this:

    # make a challenge
    random_string = random(32)
    time_value = time.time() # any string representing a timestamp will do
    challenge_key = challenge_key from account
    salt_challenge = salt_challenge from account
    aes_iv = sha256(uuid()).digest()[:16]
    cipher = aes256cfb(key=challenge_key, iv=iv, segment_width=128)
    challenge = cipher.encrypt(random_string)

    # compute the answer we antipate from the client 
    answer_cipher = aes256cfb(key=challenge, iv=iv, segment_width=8)
    time_value_ciphertext = cipher.encrypt(time_value)
    expected_answer_digest = sha256(time_value_ciphertext).digest()
    
    Persist the challenge to the database in the challenge table, so that we
    can check the challenge response when it comes in.  Get the challenge_id
    back for the inserted row.

    challenge_identifier = public_id(challenge_id)

    The server returns this as the challenge response:

    {
        challenge_id: challenge_identifier,
        challenge: base64(challenge),
        salt_challenge: base64(salt_challenge),
        iv: base64(aes_iv),
        time_value: time_value
    }
    
 Step 2: construct an answer to the challenge

    The way this works, is that we prove that given the salt_challenge, we can
    calculate challenge_key (using our passphrase) and then make use of it to
    encrypt an answer back to the server.

    In this case, we're deriving the key, and then encrypting time_value using
    that key, and sending the ciphtertext back to the server.

    challenge_key = kdf(salt_challenge, passphrase) 
    cipher = aes256cfb(key=challenge_key, iv=iv, segment_width=128)
    challenge = cipher.decrypt(challenge)

    cipher = aes256cfb(key=challenge, iv=iv, segment_width=8)
    time_value_ciphertext = cipher.encrypt(time_value)
    
    Post an answer to the challenge like this:

    {
        challenge_id: challenge_id,
        answer: base64(time_value_ciphertext),
    }

  If we have sucessfully answered the challenge, the server should then respond
  with our account info and a session token.

_end_crypto_

Performs the necessary handshakes with the server, and calls back with a potentiall empty `error` object and a conditional `session` argument.

### crypton.resurrect(sessionString, callback)

Reconstructs a serialized session and pings the server to check its validity.

## Sessions

### session.on(eventName, callback)

Session objects are event emitters. `on()`

### session.off(eventName)

Removes all event listeners for a given `eventName`.

### session.serialize(callback)

Stringifies said session object for later use, and calls back with a potentially empty `error` argument and a conditional `session` string argument.

### session.ping(callback)

Hits the appropriate route on the server to check for the validity of said session. Calls back with a potentially empty `error` argument. If the argument is empty, the session is still valid.

### session.load(containerName, callback)

_crypto_
_end_crypto

Checks for a cached `container` that is available to said session. If the `container` is not cached or a more current version is available, the latest version is retreived from the server and cached.

### session.create(containerName, callback)

_crypto_

Attempts to create a container, checking with the server to see if the namespace is available for the current session. Calls back with a potentially empty `error` argument.

Container names are encrypted and their plaintext is unknown to the server.

## Accounts

### session.account

An object containing a representation of the account associated with said session.

Example structure:

````javascript
{
  username: String,
  passphrase: String,
  keys: { },
  save: Function,
  refresh: Function,
  version: Function
}
````

### session.account.save(callback)

_crypto_
_diff_

Determines the differences between the previous version of the account of said session and saves it with the server. Calls back with a potentially empty `error` argument.

### session.account.refresh(callback)

_crypto?_

Checks with the server for new account versions from other devices. Calls back with a potentially empty `error` argument. If the argument is empty, the account has been updated.

### session.account.version

Holds the current version hash of the session.

## Transactions

Transactions are how all data is moved to the server. All `save()` methods transparently construct a transaction and commit it. 

### tx = session.transaction.create()

### tx.chunks

An array holding the `save()`d chunks of the transaction.

### tx.save(container, object, [container1, container2], ...)

Adds a chunk of data to the server. Accepts an abitrary amount of arguments containing containers and/or their objects in any format.

### tx.commit(callback)

_crypto_
_diff_

Finalizes the transaction, determines differences in containers, encrypts the data, and sends it to the server. Calls back with a potentially empty `error` argument. If the argument is empty, the transaction has been committed.

## Containers

### container.get(objectName, callback)

_crypto_
_undiff_

### container.save(callback)

_crypto_
_diff_

Determines the differences with the previously saved version of the container. Calls back with a potentially empty `error` argument. If the argument is empty, the container has been committed to the server.

### container.add(key, value)

Adds a magic key to said container.

### container.version

Holds the current version hash of the container.

### container.getHistory(callback)

Hits the server for a list of known version identifiers. Calls back with a potentially empty `error` argument and conditionally a `history` argument containing an array of version identifiers.

### container.getDiff(callback)

Constructs a Diff object containing the changes with the last known version of the container. Calls back with a potentially empty `error` argument, and conditionally a `diff` argument.

## Messages

### session.inbox.poll(callback)

### session.inbox.list(callback)

### session.inbox.filter()

### session.inbox.get()

### session.inbox.delete()

### session.inbox.clear()

### session.inbox.on()

### message.headers

### message.body

### message.delete()

## Peers

### session.getPeer()

### peer.sendMessage()

````
function uuid_factory() {
    // build a function that generates sequential uuids that are difficult to
    // cause collisions, but don't require constant streoam of new entropy
    private_counter = 1
    initial_timestamp = timestamp
    initial_random = random(32)
    function uuid2() {
        private_counter += 1
        uuid = sha256(initial_random + initial_timestamp + private_counter).hexdigest()
        return uuid
    }
    return uuid2
}

// give us a uuid closure
uuid = uuid_factory()

// this is memoizable based on peer name or maybe peer private key id
function session_key_ciphertext_for_peer(peer) {
    session_key = random(32)
    hmac_key = random(32)
    session_key_ciphertext = peer.publickey.encrypt(session_key)
    hmac_key_ciphertext = peer.publickey.encrypt(hmac_key)
    return {session_key, session_key_ciphertext, hmac_key_ciphertext}
}

session_key_ciphertext = session_key_ciphertext_for_peer(peer)
# how to send the first message...
headers_iv = sha256(uuid()).digest()[:16]
body_iv = sha256(uuid()).digest()[:16]

headers = {}
headers_plaintext = zlib.compress(json.stringify(headers))
headers_cipher = aes256cfb(key=session_key, iv=headers_iv)
headers_ciphertext =  headers_cipher.encrypt(headers_plaintext)

body = {}
body_plaintext = zlib.compress(json.stringify(body))
body_cipher =  aes256cfb(key=session_key, iv=body_iv)
body_ciphertext = body_cipher.encrypt(body_plaintext)

message_signature_hash = sha256(headers_ciphertext + headers_plaintext).digest()
message_signature = account.private_key.sign(message_signature_hash)

post this:
{
    peer_name: ...
    session_key_ciphertext:
    headers_iv:
    body_iv:
    headers_ciphertext:
    body_ciphertext:
    message_signature:
}
````

### peer.share()

### peer.unshare()

# Server

The server is a simple REST server running on node. The default all bodies are sent and received with JSON. The default success response is:

````javascript
{
  success: true
}
````

## Account

### POST /account

Creates a new account with client-generated data.

Required body:

````javascript
{

}
````

Sets session_identifier cookie (logs you in immediately) upon successful request.

### POST /account/:username

Logs into account and sets `session_identifier` cookie.

Required body:

````javascript
{

}
````

### POST /account/:username/password

Changes the password for an account.

Required body:

````javascript
{
  password: String
}
````

## Session

### GET /session

Pings the server to verify that the session is still valid. Must send `session_identifier` cookie.

If the session is invalid when an authentication-requiring route is requested, the default response will be:

````javascript
{
  success: false,
  error: "Not logged in"
}
````

## Transaction

### POST /transaction

Generates and sets `transaction_token` cookie.

Requires `session_identifier` cookie.

### POST /transaction/:token/commit

Commit (finalize) the transaction.

Requires `session_identifier` cookie.

May return the following:

````javascript
{
  success: false,
  error: "Transaction token invalid"
}
````

### DELETE /transaction/:token

Cancel a transaction without committing it to the server.

Requires `session_identifier` cookie.

May return the following:

````javascript
{
  success: false,
  error: "Transaction token invalid"
}
````

## Container

### GET /container/:container_name_ciphertext

Returns all headers of the records in the container.

Requires `session_identifier` cookie.

Optional parameter `?after=record_version_identifier` will only return the headers for records occuring after said `record_version_identifier`

Example:

````javascript
{

}
````

### POST /container/:container_name_ciphertext

`multipart/form-upload` of json + payload for this modification
// TODO fail early if the transaction is borked

Requires `session_identifier` cookie.

A valid transaction token is required or the route will return the following:

````javascript
{
  success: false,
  error: "Transaction token invalid"
}
````

### GET /container/:container_name_ciphertext/:record_version_identifier

Returns binary data of the ciphertext from the given `record_version_identifier` of the enciphered `container_name`.

Requires `session_identifier` cookie.

## Messages

### GET /inbox

Returns list of message headers as JSON objects.

Requires `session_identifier` cookie.

Optional parameters of `from=username` and `since=timestamp` may be used to filter.

Example response:

````javascript
{

}
````

### GET /inbox/:message_identifier

Returns headers and ciphtertext of payload of message with matching `message_identifier`

Requires `session_identifier` cookie.

Example response:

````javascript
{

}
````

### DELETE /inbox/:message_identifier

Deletes a given message by `message_identifier`

Requires `session_identifier` cookie.

May return the following:

````javascript
{
  success: false,
  error: "Transaction token invalid"
}
````

### POST /outbox

Send a message by `multipart/form-upload` of json + payload
// TODO fail early if the transaction is borked

Requires `session_identifier` cookie.

Example post data:

````javascript
// TODO decide on format
````

Requires session_identifier cookie.

May return the following:

````javascript
{
  success: false,
  error: "Transaction token invalid"
}
````
