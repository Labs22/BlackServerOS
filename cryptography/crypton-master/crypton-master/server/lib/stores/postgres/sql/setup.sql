begin;
create sequence version_identifier;
COMMENT ON SEQUENCE version_identifier IS
'One sequence to rule them all!

Most tables get their ID numbers from this sequence. There''s no harm in
multiple tables pulling ID numbers from the same sequence.  "God made a lot of
integers" and all that.

In a large horizontally scaling deployment, ID numbers may come from a pool of
dedicated ID servers, using something like the unified ID factory that
Nimbus.io uses (which is inspired by Instragram, which is in turn inspired by
Twitter''s Snowflake.)  For a typical site, a single PostgreSQL sequence
providing 8 byte integers is more than sufficient.
https://nimbus.io/dev/trac/browser/Nimbus.IO/tools/id_translator.py?rev=5f0f3e26389e9fc75c95ddc5de9599eefef0ec7a

What''s useful about having unique integers across the whole population of
tables is that we can pass an encoded from of these intentifiers publicly
(without leaking information about the real ID numbers) by implementing these
functions:

 server side secret keys used by public_id() and internal_id():
     id_aes_key (32 random bytes)
     id_iv_key (32 random bytes)
     id_hmac_key (32 random bytes)

 public_id(internal_id):
    a server side function to make public identifier strings that are encrypted
    and authenticated forms of internal IDs.  internal_id is any 8 byte
    integer.

    This works by encrypting the internal ID with aes256cfb(key=id_aes_key,
    segment_width=8) and including a SHA256 HMAC.

 internal_id(public_id):
    a server side function to authenticate and decrypt a public ID back to an
    internal 8 byte integer ID

In these comments, I will attempt to use "_id" suffix to describe the internal
ID (which is an 8 byte integer) and "_identifier" to describe the public
version of an ID (which is a string.)  Only the server can translate
identifiers to IDs.

These are similar to Nimbus.io''s ID Translator.
    https://nimbus.io/dev/trac/browser/Nimbus.IO/tools/id_translator.py?rev=5f0f3e26389e9fc75c95ddc5de9599eefef0ec7a
';
CREATE TABLE account (
    account_id int8 not null primary key default nextval('version_identifier'),
    creation_time timestamp not null default current_timestamp,
    username text unique not null,
    base_keyring_id int8,
    deletion_time timestamp
);
COMMENT ON TABLE account IS
'basic account info';

COMMENT ON COLUMN account.account_id IS
'Like many IDs used in this project, this ID can be made into a public ID
(which is an opague string, decryptable and HMACable by the server.)  When the
client side framework refers to an account, it should do so by this public form
of account_id.  This makes references to an account stable across username
changes and such.';

CREATE TABLE base_keyring (
    base_keyring_id int8 not null primary key default nextval('version_identifier'),
    account_id int8 not null references account,
    creation_time timestamp not null default current_timestamp,
    srp_verifier bytea,
    srp_salt bytea,
    keypair_salt bytea,
    keypair_mac_salt bytea,
    keypair bytea,
    keypair_mac bytea,
    pubkey bytea,
    container_name_hmac_key bytea,
    hmac_key bytea,
    sign_key_pub bytea,
    sign_key_private_mac_salt bytea,
    sign_key_private_ciphertext bytea,
    sign_key_private_mac bytea,
    deletion_time timestamp
);

COMMENT ON TABLE base_keyring IS
'Base set of keys used by an account.
Some of these change whenever an account changes the passphrase.
These are stored separately from account, even though only one base_keyring
should be active at any given time.';
COMMENT ON COLUMN base_keyring.srp_verifier IS
'SRP verifier data used to authenticate the client';
COMMENT ON COLUMN base_keyring.srp_salt IS
'Salt used for SRP authentication';
COMMENT ON COLUMN base_keyring.keypair_salt IS
'Salt used with KDF and passphrase to create AES256 key used to encrypt keypair';
COMMENT ON COLUMN base_keyring.keypair_mac_salt IS
'Salt used with KDF and passphrase to create HMAC key used to validate keypair';
COMMENT ON COLUMN base_keyring.keypair IS
'AES ciphertext of serialize public/private keypair';
COMMENT ON COLUMN base_keyring.keypair_mac IS
'HMAC of the public/private keypair ciphertext';
COMMENT ON COLUMN base_keyring.pubkey IS
'Plaintext of serialized public key from the keypair';
COMMENT ON COLUMN base_keyring.container_name_hmac_key IS
'AES output ciphertext of 32 byte HMAC key used for container names';
COMMENT ON COLUMN base_keyring.hmac_key IS
'AES output ciphertext of 32 byte HMAC key for general data authentication';
COMMENT ON COLUMN base_keyring.sign_key_pub IS
'ECDSA public key used by Alice to verify Bobs signature';
COMMENT ON COLUMN base_keyring.sign_key_private_mac_salt IS
'Salt used with a KDF and a passphrase to authenticate the contents of the ciphertext of ECDSA private key used for autheticating public key encrypted messages, data';
COMMENT ON COLUMN base_keyring.sign_key_private_ciphertext IS
'AES output ciphertext of ECDSA private key used for autheticating public key encrypted messages, data';
COMMENT ON COLUMN base_keyring.sign_key_private_mac IS
'HMAC of the ciphertext of ECDSA private key used for autheticating public key encrypted messages, data';
create table container (
    container_id int8 not null primary key default nextval('version_identifier'),
    account_id int8 not null references account,
    name_hmac bytea not null unique,
    creation_time timestamp not null default current_timestamp,
    modified_time timestamp not null default current_timestamp,
    latest_record_id int8 not null default 0,
    deletion_time timestamp
);
COMMENT ON TABLE container IS
'A partition of the object database kept for an application user.
The state of the object database is then built up by appling records
sequentially.

How crypto for containers works:

    containers have session keys (tracked in container_session_key)

        A session key for a container involves 2 keys:
            encryption key (32 random bytes)
            HMAC key (32 random bytes.)

        The session keys are used by the accounts participating in the
        container to encrypt, decrypt, and verify container records.

        This table tracks the creation and succession of keys.  A container
        that''s only used by one user will likely have the same session key
        throughout its use.

        A container shared with many users will typically get a new session key
        (superceding the existing key) whenever a container is unshared with an
        account, so that new records in the container would no longer be
        readable by the unshared account.

    session keys are shared with accounts (in container_session_key_share)

        Critically, the keys themselves are NOT STORED directly in
        container_session_key, but rather stored 1:many in
        container_session_key_share, encrypted to each account that should be
        able to read the records stored with the session keys.

        For every entry in container_session_key, there will be one or more
        records in container_session_key_share.

    container data is a series of records (in container_record)

        The state of the container is built up by the client by applying all
        the container''s records in order.

        Each record is encrypted by applying AES256CFB, w/ a random 16 byte IV,
        and with segment width of 128 bits and simple pkcs7 padding.  The key
        used is the current container_session_key.  An HMAC-SHA256 of the
        resulting ciphertext using the session''s HMAC key is also stored.

    So the process of creating a new container is:
        - determine the container_name_ciphertext: by taking the normalized utf8
            HMAC-SHA256(key=container_name_hmac_key,
                        msg=norm_utf8(name_plaintext)
        - generate container_session keys
            session_key = random(32)
            hmac_key = random(32)
            signature = private_key.sign(
                hmac(key='', msg=session_key + hmac_key).digest())
        - for every account (including the owner) that the container is shared
          with, calculate the container_session_key_share entry.
            session_key_ciphertext is session_key encrypted to to_account_id''s
                public key (using PKCS#1)
            hmac_key_ciphertext is hmac_key encrypted to to_account_id''s
                public key (using PKCS#1)
        - create a transaction
        - add container entry to transaction
        - add container_session_key entry to transaction
        - add one or more container_session_key_share entries to transaction
            (one entry for every account we''re sharing the key with, including
            the container''s creator, as calculated above.)
        - add one or more container_record entries to the transaction
        - (add any other data to the transaction needed, including more
          containers, messages, whatever.)
        - ask the server to complete the transaction
        - handle any errors the server returns regarding completing the
          transactions. (typical errors will involve our local state being too
          old, and needing a refresh/retry.)
';
COMMENT ON COLUMN container.name_hmac IS
'This is an HMAC of the name the application chose for the container.  This
happens automatically by the framework, allowing the developer to use plaintext
names.  It is calculated using container_name_hmac_key.  The server never knows
the plaintext of container names.  These are enforced to be globally unique
across all accounts because collisions are unlikely.

When one user shares a container with another user, or messages another user
referencing a named container, the communication must refer to the container by
name_hmac, since the server should not know (and will have no way to lookup)
containers based on their plaintext names.

The system does not guarantee that deleted container names are recyleable. In
other words, if you create a container, then delete it, you might not be able
to create another container with the same name.  An common application level
strategy is to have a small top level container that keeps track of the name of
other containers you use.';

COMMENT ON COLUMN container.latest_record_id IS
'This is a denormalized shortcut column to allow the server to easily determine
if a client does or does not have the latest records in a container.  Otherwise
it would have to do a slower query on container_record.  This should be updated
to be the largest record_id whenever new records are added.  Transactions that
add multiple records to a container should update the container to the largest
new record_id.';
COMMENT ON COLUMN container.modified_time IS
'Containers can be modified by adding records, or by changing ACLs of the
container.  Either change should update this column to latest timestamp.';

create table container_session_key (
    container_session_key_id int8 not null primary key
        default nextval('version_identifier'),
    container_id int8 not null references container on delete cascade,
    account_id int8 not null references account,
    transaction_id int8 not null,
    creation_time timestamp not null default current_timestamp,
    signature bytea not null,
    supercede_time timestamp
);
CREATE UNIQUE INDEX container_session_key_active_idx
    ON container_session_key (container_id)
    WHERE supercede_time IS NULL;
COMMENT ON INDEX container_session_key_active_idx IS
'Enforce that for any given container only one session_key is active.  So to
change the session key, you must first mark previous session keys with a
supercede_time.';

COMMENT ON TABLE container_session_key IS
'Note that the actual session key isn''t stored by records in this table. It''s
stored one or more times, encrypted to the accounts that should be able to read
it, in container_session_key_share.  Naturally the owner''s account should be
one of them, so that the owner may read their own container!

There is a view below readable_container_records_by_account that
demonstrates finding the data records in a container that any given account
should have access to.';
COMMENT ON COLUMN container_session_key.account_id IS
'This is the account setting the session key -- not necessarily the account
that owns the container.';

COMMENT ON COLUMN container_session_key.signature IS
'Private key signature of the HMAC of the plaintext of the session key + hmac
key';
COMMENT ON COLUMN container_session_key.supercede_time IS
'This is the time when a session_key is replaced by a new session key. This
happens when a container is unshared, so that new records added into the
container are not readable to accounts which had access to the previous session
key.';

create table container_session_key_share (
    container_session_key_share_id int8 not null primary key
        default nextval('version_identifier'),
    container_session_key_id int8 not null references container_session_key on delete cascade,
    account_id int8 not null references account,
    to_account_id int8 not null references account (account_id),
    transaction_id int8 not null,
    session_key_ciphertext varchar not null,
    deletion_time timestamp
);

COMMENT ON TABLE container_session_key_share IS
'Make one of a container''s session keys readable to an account';
COMMENT ON COLUMN container_session_key_share.session_key_ciphertext IS
'This is the output of encrypting the AES256 session key to the public key
owned by to_account_id.';
COMMENT ON COLUMN container_session_key_share.deletion_time IS
'When a container is unshared, deletion_time for records that shared a
containers keys with that user are set. (and also new keys are set for future
records.)';

create table container_record (
    container_record_id int8 not null primary key
        default nextval('version_identifier'),
    container_id int8 not null references container on delete cascade,
    container_session_key_id int8 not null references container_session_key,
    account_id int8 not null references account,
    transaction_id int8 not null,
    creation_time timestamp not null default current_timestamp,
    payload_ciphertext varchar not null
);

COMMENT ON TABLE container_record IS
'These are the encrypted records that build up state for the object database
inside a container.  Each record has a payload which is a compressed
serialization of some change of state inside the container.';
COMMENT ON COLUMN container_record.account_id IS
'This is the account that added this record.  For a single user container, this
column is redundant, since we already have container_id which gives us an
account_id.  However, when we have containers that allow modifications by
multiple users, this tells us which user made the modification.';
COMMENT ON COLUMN container_record.transaction_id IS
'Transaction ID that added this record.';

CREATE OR REPLACE VIEW readable_container_records_by_account AS
    SELECT container_record.*,
           container_session_key.signature,
           container_session_key_share.to_account_id,
           container_session_key_share.session_key_ciphertext
      FROM container_record
      JOIN container_session_key using (container_session_key_id)
      JOIN container_session_key_share using (container_session_key_id)
     WHERE container_session_key.supercede_time IS NULL
       AND container_session_key_share.deletion_time IS NULL
  ORDER BY container_record.creation_time ASC;
COMMENT ON VIEW readable_container_records_by_account IS
'An example of a query to find records in a container that are readable by
various accounts.  You could select with "WHERE to_account_id=?" to filter the
results down to those records that are readable by a particular account.';


/*
Client / Server Communication:
    Creating An Account
    Authentication to Account
    Posting a Transaction
    Retrieving Data
    POST /account (to create new account)
        sets session_identifier cookie (logs you in immediately)
    POST /account/:username (to auth into an account)
        sets session_identifier cookie
    POST /account/:username/password (to change the password for an account)
    GET /session (ping to server to verify that the session is still valid).
        returns { success: true } or { success: false, error: "you suck" }
    POST /transaction
        set transaction_token cookie
    POST /transaction/:token/commit
        commit the transaction, returns { success: true } or { success: false, error: "you suck" }
    DELETE /transaction/:token
        rollback the transaction
    GET /container/:container_name_ciphertext
        all the headers of the records in the container
    GET /container/:container_name_ciphertext?after=:record_version_identifier
        all the headers of the records in the container since some version identifier
    POST /container/:container_name_ciphertext
        (fails if there is no transaction_token cookie set)
        multipart/form-upload of json + payload for this modification // TODO fail early if the transaction is borked
    GET /container/:container_name_ciphertext/:record_version_identifier
        returns binary data of the ciphertext from the container's record.
    GET /inbox
        returns list of message headers as json objects
    GET /inbox?from=:username , since=:time , ....
        filter list of message headers as json objects by various things
    GET /inbox/:message_identifier
        returns ciphtertext of payload of the message
    DELETE /inbox/:message_identifier
        (fails if there is no transaction_token cookie set)
        returns ciphtertext of payload of the message
    POST /outbox
        (fails if there is no transaction_token cookie set)
        multipart/form-upload of json + payload for this message // TODO fail early if the transaction is borked


*/

create table message (
    message_id int8 not null default nextval('version_identifier'),
    creation_time timestamp not null default current_timestamp,
    ttl interval,
    from_account_id int8 not null references account (account_id),
    to_account_id int8 not null references account (account_id),
    headers_ciphertext varchar not null,
    payload_ciphertext varchar not null,
    deletion_time timestamp
    constraint deleted_after_created
        check (deletion_time is null or deletion_time >= creation_time)
/*
    constraint header_ciphertext_len_modulo
        check (octet_length(header_ciphertext) % 16 = 0)
    constraint header_ciphertext_len
        check (octet_length(header_ciphertext) BETWEEN 16 and 4096)
    constraint payload_ciphertext_len_modulo
        check (octet_length(payload_ciphertext) % 16 = 0)
    constraint payload_ciphertext_len
        check (octet_length(payload_ciphertext) BETWEEN 16 and 1048576)
*/
);

COMMENT ON TABLE message IS 'realtime messages between accounts';
COMMENT ON COLUMN message.ttl IS
'Optional field to denote a message as transient, having an end time.  Messages
may be automatically deleted by the server after this interval.';
COMMENT ON COLUMN message.deletion_time IS
'the time the server marks a message as having been deleted by the recipient.
rows for deleted messages may optionally also be deleted entirely.';
COMMENT ON COLUMN message.headers_ciphertext IS
'AES-GCM of header, key=hash(data key)';
COMMENT ON COLUMN message.payload_ciphertext IS
'AES-GCM of payload, key=hash(data key)';

create table transaction (
    transaction_id int8 not null primary key default nextval('version_identifier'),
    account_id int8 not null,
    creation_time timestamp not null default current_timestamp,
    modify_time timestamp,
    num_operations int4 not null default 0,
    abort_timestamp timestamp,
    commit_request_time timestamp,
    commit_start_time timestamp,
    committer_hostname text,
    committer_pid int8,
    commit_finish_time timestamp,
    success bool,
    errors text /* change to json for pg 9.2 */
);

create view first_pending_commit_per_avatar as
select *
  from transaction t1
 where commit_request_time is not null
   and commit_start_time is null
   and not exists (select 1
                     from transaction t2
                    where commit_request_time is not null
                      and commit_start_time is null
                      and t2.transaction_id <> t1.transaction_id
                      and t2.account_id = t1.account_id
                      and t2.commit_request_time < t1.commit_request_time
                    limit 1);

CREATE INDEX transaction_commit_active ON transaction (account_id)
    where commit_start_time IS NOT NULL AND commit_finish_time IS NULL;
CREATE INDEX transaction_commit_waiting ON transaction (account_id)
    where commit_request_time IS NOT NULL AND commit_start_time IS NULL;
CREATE VIEW commitable_transaction as
    SELECT transaction.*
      FROM transaction
     WHERE commit_request_time IS NOT NULL AND commit_start_time IS NULL
       AND NOT EXISTS (SELECT 1
                         FROM transaction tx2
                        WHERE tx2.account_id=transaction.account_id
                          AND commit_start_time is not null
                          AND commit_finish_time is null)
  ORDER BY commit_request_time;

COMMENT ON TABLE transaction IS
'These are application level transactions applied on top of SQL transactions.
Transactions are built up by the application, by way of creating a row in the
transaction table, then adding rows to associated tables
(transaction_add_account, transaction_add_container_record, etc.) to build up
the content of the transaction.  Finally, the application either aborts a
transaction (setting transaction.abort_timestamp) or requests that the
transaction be committed (setting transaction.commit_request_time)

Once a commit is requested, a server side worker will process the transaction.
It will immediately set commit_start_time, committer_hostname, and
committer_pid to indicate that work has started in committing the transaction.

Only one transaction maybe applied concurrently per account (account level
commit lock.)

Only one transaction maybe applied concurrently per container (container level
lock.)  If a transaction modifies many containers, the worker applying the
transaction must get the locks for all containers in consistent sorted order to
avoid deadlock between multiple transactions.  Workers should obtain container
locks before account locks.

For scalability, transactions could be separated away from the rest of the
database (in other words, the database servers providing the transaction
related tables do not need to be the same servers and same database as those
holding the main tables.)  Transactions could be sharded across many database
servers, partitioning perhaps by account_id.  Since transactions are built up
in their own durable database, and applying a transaction is an idempotent
operation, it is also straight forward to further shard a large site database
partitioning by container.

Here are some rules for validating a transaction before committing it:

If a transaction adds an account:
    It must also add exactly one base_keyring.
If a transaction modifies a container through any of the associated tables:
    All records from all tables must have the same
        latest_record_id value.
    That latest_record_id value must match container.latest_record_id.
    Otherwise the transaction cannot be committed, with the error that at least
    this container must be refreshed.
If a transaction deletes a container:
    Similar to above, the deletion action must have latest_record_id matching
    container.latest_record_id.  Otherwise the requestor is told that the
    transaction is out of date with regard to that container.
If a transaction deletes a message:
    It is specifically NOT AN ERROR if that message does not exist or is
    already deleted (i.e. deleting messages does not require strict data
    integrity.)
If a transaction deletes a container_session_key_share:
    It is specifically NOT AN ERROR if that container_session_key_share does
    not exist. (i.e. deleting container_session_key_share does not require
    strict data integrity.)
If a transaction creates a container:
    It must also add a container_session_key for that container.
    It must also add a container_session_key_share for that session key,
    to_account_id pointing to the creator''s account.  (It may add others as
    well.)
If a transaction deletes a container_session_key_share:
    (This happens when a container in unshared.)
    It must also add a new container_session_key for that container.
    It must also add a container_session_key_share for that session key,
    to_account_id pointing to the creator''s of the transaction''s account.
        (It may add others as well.)
    For all accounts that were able to read previous records, it must either
    also delete container_session_key_share for those users, or add new
    container_session_key_share entries for them to read the new session_key.
    In other words, accounts should be able to read all of the container or
    none of it.  You wouldn''t want to change the session_key, and omit adding
    a container_session_key_share for one of the accounts who used to be able
    to read the container.

When a commit worker begins processing a transaction, a first past of
validating all of these rules can be made before we acquire any locks.  Some of
those conditions (such as comparisons to latest_record_id) need to be checked
again after locks are acquired.  Checking the conditions twice, first without
acquiring any locks, probably gives us slower overall commit speed, but may
greatly decrease the worst-case latency by narrowing lock contention.

';

create table transaction_add_container (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null references transaction,
    name_hmac bytea not null
);

/* disallow adding the same container name twice in the same transaction */
create unique index transaction_add_container_tx_name_idx
    on transaction_add_container (transaction_id, name_hmac);

create table transaction_delete_container (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null references transaction,
    name_hmac bytea not null,
    latest_record_id int8 /* not null */
);
/* disallow deleting the same container name twice in the same transaction */
create unique index transaction_delete_container_tx_name_idx
    on transaction_delete_container (transaction_id, name_hmac);

create table transaction_add_container_session_key (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null,
    name_hmac bytea not null,
    latest_record_id int8,
    signature bytea not null,
    supercede_key int8
);
/* disallow adding more than one session key for the same container in the same
 * transaction */
create unique index transaction_add_container_session_key_tx_name_idx
    on transaction_add_container_session_key (transaction_id, name_hmac);

create table transaction_add_container_session_key_share (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null references transaction,
    name_hmac bytea not null,
    latest_record_id int8,
    to_account_id int8 not null,
    session_key_ciphertext varchar not null
);
/* disallow adding more than one session key share for the same container to
 * the same account in the same transaction */
create unique index transaction_add_container_session_key_share_name_acct_idx
    on transaction_add_container_session_key_share
    (transaction_id, name_hmac, to_account_id);

create table transaction_delete_container_session_key_share (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null references transaction,
    name_hmac bytea not null,
    latest_record_id int8,
    container_session_key_id int8 not null,
    to_account_id int8 not null
);
/* disallow deleting a session key share for the same container to the same
 * account more than once per transaction */
create unique index transaction_delete_container_session_key_share_acct_idx
    on transaction_delete_container_session_key_share
    (transaction_id, name_hmac, to_account_id);

create table transaction_add_container_record (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null references transaction,
    name_hmac bytea not null,
    latest_record_id int8,
    payload_ciphertext varchar not null
);

create table transaction_add_message (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null references transaction,
    ttl interval,
    to_account_id int8 not null,
    keys_ciphertext bytea not null,
    keys_signature bytea not null,
    header_ciphertext bytea not null,
    payload_ciphertext bytea not null
);

create table transaction_delete_message (
    id int8 not null primary key default nextval('version_identifier'),
    transaction_id int8 not null references transaction,
    message_id int8 not null
);
/* disallow deleting a message_id more than once per transaction */
create unique index transaction_delete_message_msg_id_idx
    on transaction_delete_message
    (transaction_id, message_id);


create table item (
    item_id int8 not null primary key default nextval('version_identifier'),
    account_id int8 not null references account,
    name_hmac bytea not null unique,
    creation_time timestamp not null default current_timestamp,
    modified_time timestamp not null default current_timestamp,
    deletion_time timestamp,
    value bytea,
    timeline_visible boolean default false
);

create unique index item_name_hmac_idx
    on item (name_hmac);

create table item_history (
    item_history_id int8 not null primary key default nextval('version_identifier'),
    item_id int8 not null references item,
    account_id int8 not null references account,
    creation_time timestamp not null default current_timestamp,
    modified_time timestamp not null default current_timestamp,
    deletion_time timestamp,
    value bytea
);

create table timeline (
    timeline_id int8 not null primary key default nextval('version_identifier'),
    creator_id int8 not null references account,
    receiver_id int8 not null references account,
    item_id int8 not null references item,
    creation_time timestamp not null default current_timestamp,
    modified_time timestamp not null default current_timestamp,
    deletion_time timestamp,
    value bytea
);

create table item_session_key (
    item_session_key_id int8 not null primary key
        default nextval('version_identifier'),
    item_id int8 not null references item on delete cascade,
    account_id int8 not null references account,
    creation_time timestamp not null default current_timestamp,
    supercede_time timestamp
);

CREATE UNIQUE INDEX item_session_key_active_idx
    ON item_session_key (item_id)
    WHERE supercede_time IS NULL;
-- COMMENT ON item_session_key_active_idx IS
-- 'Enforce that for any given item only one session_key is active.  So to
-- change the session key, you must first mark previous session keys with a
-- supercede_time.';

-- COMMENT ON TABLE item_session_key IS
-- 'Note that the actual session key isn''t stored by records in this table. It''s
-- stored one or more times, encrypted to the accounts that should be able to read
-- it, in item_session_key_share.  Naturally the owner''s account should be
-- one of them, so that the owner may read their own container!'

-- COMMENT ON COLUMN item_session_key.account_id IS
-- 'This is the account setting the session key';

-- COMMENT ON COLUMN item_session_key.supercede_time IS
-- 'This is the time when a session_key is replaced by a new session key. This
-- happens when an item is unshared.';

create table item_session_key_share (
    item_session_key_share_id int8 not null primary key
        default nextval('version_identifier'),
    item_session_key_id int8 not null references item_session_key on delete cascade,
    account_id int8 not null references account,
    to_account_id int8 not null references account (account_id),
    session_key_ciphertext varchar not null,
    deletion_time timestamp
);

COMMENT ON TABLE item_session_key_share IS
'Make one of a item''s session keys readable to an account';
COMMENT ON COLUMN item_session_key_share.session_key_ciphertext IS
'This is the output of encrypting the AES256 session key to the public key
owned by to_account_id.';
COMMENT ON COLUMN item_session_key_share.deletion_time IS
'When an item is unshared, deletion_time';

CREATE OR REPLACE FUNCTION notifyUpdatedItem() RETURNS TRIGGER AS $$
  DECLARE 
    notify_row RECORD;
  BEGIN
    FOR notify_row IN 
      SELECT s.item_session_key_share_id, 
        s.account_id, s.to_account_id, k.item_id, 
	  a.username AS toUser, b.username AS fromUser 
        FROM item_session_key_share s 
        JOIN item_session_key k ON 
          (s.item_session_key_id = k.item_session_key_id)
        JOIN account a ON 
          (s.to_account_id = a.account_id)
        JOIN account b ON 
          (s.account_id = b.account_id)
        WHERE k.item_id = NEW.item_id AND k.supercede_time IS NULL 
    LOOP
      PERFORM pg_notify('SharedItemUpdated', 
        CAST(notify_row.to_account_id AS text)|| ' ' ||
        CAST(notify_row.account_id AS text) || ' ' ||
        encode(NEW.name_hmac, 'escape') || ' ' ||  
        notify_row.toUser || ' ' || 
	notify_row.fromUser);

    END LOOP;
    RETURN NULL;
  -- XXXddahl: EXCEPTION RAISE pg_notify notification???
  END;
$$ LANGUAGE PLPGSQL;

CREATE TRIGGER UpdatedItemNotify AFTER UPDATE ON item FOR EACH ROW EXECUTE PROCEDURE notifyUpdatedItem();

CREATE OR REPLACE FUNCTION populateItemHistoryInsertUpdate() RETURNS TRIGGER AS $$
  BEGIN
    INSERT INTO item_history (item_id, account_id, creation_time, value)
    VALUES (NEW.item_id, NEW.account_id, NEW.creation_time, NEW.value);  	
    RETURN NULL;
  END;
$$ LANGUAGE PLPGSQL;

CREATE TRIGGER PopulateItemHistoryInsert AFTER INSERT ON item FOR EACH ROW EXECUTE PROCEDURE populateItemHistoryInsertUpdate();

CREATE TRIGGER PopulateItemHistoryUpdate AFTER UPDATE ON item FOR EACH ROW EXECUTE PROCEDURE populateItemHistoryInsertUpdate();

-- We need to insert a timeline record for each item / item_key_share pair insert/update event

CREATE OR REPLACE FUNCTION populateTimeline() RETURNS TRIGGER AS $$
  DECLARE 
    item_row RECORD;
  BEGIN
    FOR item_row IN 
      SELECT s.item_session_key_share_id, 
        s.account_id, s.to_account_id, k.item_id, 
	  a.username AS toUser, b.username AS fromUser 
        FROM item_session_key_share s 
        JOIN item_session_key k ON 
          (s.item_session_key_id = k.item_session_key_id)
        JOIN account a ON 
          (s.to_account_id = a.account_id)
        JOIN account b ON 
          (s.account_id = b.account_id)
        WHERE k.item_id = NEW.item_id AND k.supercede_time IS NULL
	  AND NEW.timeline_visible = TRUE
    LOOP
      -- Insert a timeline row for each session_key_share
      INSERT INTO timeline (item_id, creator_id, receiver_id, creation_time, value)
      VALUES (NEW.item_id, item_row.account_id, item_row.to_account_id, NEW.creation_time, NEW.value);

    END LOOP;
    RETURN NULL;
  END;
$$ LANGUAGE PLPGSQL;

CREATE TRIGGER PopulateTimelineForEachItemInsert AFTER INSERT ON item FOR EACH ROW EXECUTE PROCEDURE populateTimeline();

CREATE TRIGGER PopulateTimelineForEachItemUpdate AFTER UPDATE ON item FOR EACH ROW EXECUTE PROCEDURE populateTimeline();

commit;
