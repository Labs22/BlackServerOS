# synopsis spec of zero knowledge application framework.

# this is a draft. no code exists. just trying to make the interface intuitive
# and effective.  But I already wish, this had existed 5 years ago when I was
# writing the original SpiderOak!

# this demos client side operations, so everything is from the perspective of
# code running in a browser.  We'll demo the ways a developer would use the
# framework to build apps, rather than show the internal details of what the
# framework is doing behind the scenes.  We will attempt to descirbe those
# behind the scenes details (mostly about crypto, protocol, and internal data
# structures) in comments.

# we're using pseudocode rather correct javascript, because for purposes of
# understanding and iterating the concept quickly, I find this easier to read.
# Also, the spec is intended to be language agnostic, such tha clients could be
# developed in a variety of languages, and all interoperate.

# for similar reasons, this is written using synchronous code, whereas in a
# real app an async approach would usually be needed with callbacks and such,
# since many of the methods involve blocking network access, although the
# framework may do behind the scenes caching so blocking is reduced.  Since
# we're just trying to demo capabilities here and most people find synchronous
# code easier to read, we'll stick with that.

# === generating a new account (i.e. which implies a crypto context for object
# storage and communication with peers.)

# bob signs up for an account in our OMG awesome zero knowledge diary
# application.

# this locally generates an object containing root level keys, including an
# outer level session key derived from the pass phrase using a key derivation
# function.  Also creates public/private keypair, a HMAC key, a couple of salt
# strings, and a challenge key that can later be used for auth between
# client/server via a zero knowledge password proof.
account = zkaf.generate_account('pass phrase')
# account names can be whatever strings the server's policy allows. that's up
# to the server. account names are plain text.
account.name = 'account_name'
# this establishes a new account on the server with the given name. that
# name might not be available so check result.
if (result.status !== 'ok') {
    if (result.error !== 'dupe') {
        alert("can't create this account: " + result.errormessage)
        return
    }
    account.name = 'another account_name' # try again with a name that isn't taken
    result = account.save()
}

# now we have an account. we can securely store objects and data.

# === authenticating into an established account (establishing a crypto session
# in the context of an existing account.)
# this does a zero knowledge password proof auth to the server for the
# specified account, then retrieves the cipher text of our keys, decrypts them
# with the key from the KDF and the pass phrase, and therefore establishes a
# crypto session with access to our keys and authorization to retrieve our
# storage containers.
session = zkaf.auth('account_name', 'pass phrase')
if not session:
    # we don't necessarily get a clear message on why this failed, because
    # that would leak information
    raise RuntimeError("bad account name or password")

# === persisting a session
# caching the crypto session locally (HTML5 local storage?) so we don't have to
# prompt for password and derive keys again (which is expensive.)
session_as_string = session.serialize() # save this somewhere

# resurrect it later
zkaf.session.from_string(session_as_string)
# make sure that server believes this is still a valid account/session. (like
# if it's been persisted for a long time..)
result = session.ping()
# for example, the session might be invalid if the password has changed since
# it was saved, or if the account has been deleteded or disabled server side
# (such as for non-payment.)

# === changing a pass phrase for an account
session.account.pasphrase = 'new passprhase'
result = session.account.save()
# check the result -- you might get a version error if you don't have the
# newest version of the account at the time you try to save. (more on
# versioning later.)
if result == 'refresh':
    old_version = session.account.version()
    session.account.refresh()
    new_version = session.account.version()
    result = session.account.save() 

# === introducing object storage via containers 
# we're going to store our app's data using objects, like a traditional object
# database.  Many people are already familiar with object databases such as
# ZODB.  Our object database is a similar concept, while making zero knowledge
# privacy guarantees and also making it possible to selectively (and privately)
# share and collaborate with others.

# Objects are stored in containers.  Containers are always associated with a
# specific account (and unreadable by other accounts, unless explicitly shared
# with a peer or a group.)  Containers are a way of partitioning the data
# required for object storage. an app with relatively smallish data storage
# requirements might just use one container, and store every object in that
# container. other apps may use many separate containers.  A container must be
# fetched in its full length from the server and decrypted client side for the
# objects inside it to be read.  

container = session.load('diary') # load this container from the server to
                                  # local, making access to all objects
                                  # available without blocking.  If the
                                  # container is already available in cache,
                                  # this returns immediately.

# containers are identified by keys such as 'diary' above. any string is
# allowed. From the server's perspective, container names (and of course their
# contents) are unreadable.

# If the data set is going to grow very large, partitioning data across
# containers and lazy loading them as you need them can help with app load
# time.  Behind the scenes, the framework does some local caching for the
# contents of frequently used storage containers, so it doesn't always have to
# download the full container when the app needs it.

# One simple tactic for partitioning data among containers is to keep app
# metadata all in one container, and keep bulky binary data (images, videos,
# long text strings, whatever) in many other containers.  Metadata is usually
# very small, and can load quickly.

# For example, a diary app might have a single metadata container with a list
# of entries. Each entry has a title, date, keywords, and other basic
# attributes.  Average size is probably under 500 bytes.  So one entry per day
# for 10 years would only be be 1.8 meg.  In 10 years, 1.8 meg will be like 1.8
# KB today, so storing all this in one container is fine.  Each entry would
# reference other containers to find the text of the entry and media
# attachments like images, videos, etc.


diary_entries = container.get("entries")
draft_entries = container.get("drafts")
# these objects are like regular javascript objects that we can store data
# into.
new_entry = { id: get_uuid(), title: "Adventures with Crypto" }
draft_entries.push(new_entry)
# atomically save all modified objects back to the container. if we had made
# changes to either diary_entries or draft_entries, both would be saved. the
# default parameters for saving objects preserves object history (i.e. previous
# versions of the object are still reachable) and uses diffing where
# appropritae to minimize the total size. 
result = container.save()

# let's add some more content to this entry.  We'll store the text content
# separately from the metadata, in its own one-off container.
text = "we're having fun here in object storage land...."

new_entry.text_container_name = 'text_for_' + new_entry.id
text_container = session.new_container(new_entry.text_container_name)
text_container.add(new_entry.id, text)
text_container.save() # save our text to one the one-off container
container.save() # and save our changes to draft_entries from above to the main
                 # container. (later we'll show how to do both of these
                 # atomically in one op.)

# here's how we would iterate over all the entries in the diary, retrieving
# text and media attachments from their own containers.
for entry in diary_entries:
    text_container = session.load(entry.text_container_name)
    diary_text = text_container.get(entry.id)
    if entry.attachment_container_name:
        attachment_container = session.load(entry.attachment_container_name)
        list_of_attachments = attachment_container.get(entry.id)

# general container operations
list_of_object_keys = container.keys()
myobject = container.new('mynewobjectname', {})
myobject = container.get('myobjectname')
myobject = container.delete('myobjectname') # it's still available in the
# container's history, until the next compaction. more on compaction later.
myobject.new_property = 'abc'
myobject.save()
myobject.delete()

myobject = container.get('myobjectname')
mylist = []
myobject['list_of_stuff'] = mylist
mylist.push(1)



# see which objects have been locally modified and not yet saved
modified_keys = container.modified_keys()
modified_objects = container.modified_objects()

# atomically save an object, multiple objects, or all modified objects
result = myobject.save()
result = container.save([myobject])
result = container.save_keys(['myobjectname'], 
                             # some optional paramaters
result = container.save() # save everything that's been locally modified
result = container.save(  # give the container more direction on finding 
                          # modified objects. by default it will do a deep
                          # comparsion of every object that's been retrieved
                          # from the container with .get(), which is guaranteed
                          # to be accurate, but can be slow if there are many
                          # objects or very large objects.
                        { shallow = 1, deep = 1, levels = 5})

# atomically save across multiple containers using a transaction:
tx = session.tx()
tx.save(myobject)  # add a particular object to a transaction 
tx.save([myobject1, myobject2]) # add a list of objects
tx.save(container1, [another_object, and_another_object]) # add a list of objects
tx.save(container3, [another_object, and_another_object])
result = tx.commit()

if result == 'refresh':
    # refresh and deal with the changes.

# == modifying objects, concurrency, and versioning


# containers have version identifiers. the identifiers are opague strings
# provided by the server. they aren't sortable client side.  a container's
# version identifier changes whenever modifications to a container and/or the
# objects in a container are saved.  When a container is loaded, it normally
# includes the complete change history of all objects in the container (but see
# below about compaction.)  This means that clients can see the history of
# objects in a container, and find the differences between them.

# refreshing objects, finding changes between times.

old_identifier = container.version_identifier()
container.refresh()
new_identifier = container.version_identifier()
# getting many tokens at once
version_identifier_list = session.version_identifiers([container1, container2])

# we can see which things have changed between versions of a container.
if not old_identifier == new_identifier:
    list_of_changed_keys = container.modified_keys(old_identifier, new_identifier)
    # for a given key, we can get a list of one or more diff objects that shows
    # us specifically what changed between the intervening version(s)
    list_of_diffs = container.diffs(key, old_identifier, new_identifier)
    # or we could compare the objects ourselves
    old_object = container.get_version(key, old_identifier)
    new_object = container.get_version(key, new_identifier)
    # or we could see the whole history
    object_history = container.get_history(key)
    for entry in object_history:
        # we can look through the attributes of the entry

        # these properties are added by the server, and are guaranteed to be
        # correct.
        entry.version_identifier # the new version of the container at the time
                                 # this object was changed. (note that many
                                 # objects in the container may share the same
                                 # version identifiers, since the identifier
                                 # applies to the entire container and all the
                                 # objects it changes; a transaction modifying
                                 # several objects would give the new versions
                                 # of those objects each the same new version
                                 # identifier.
        entry.author    # the account name the change came from. 
        entry.timestamp # this is server time from whenever .save() was called.
                        # not the time the object actually changed. (also
                        # guaranteed to be accurate by the server.)
        entry.diffsize  # the size of the binary storage of the diff (after 
                        # compression, encryption, etc.)  In other words, the
                        # length of the ciphertext of the diff.

        # for these, the server cannot guarantee anything about the contents, since
        # they are unreadable to the server.

        entry.get_serialized_diff() # get the serialize (as a string) of the 
                                    # diff (the plaintext.)

        entry.get_diff() # get the actual diff object.

        old_object = entry.get_object() # just materialize the object at this
                                        # point in history.

        old_object.abc = "changed"  # this change is temporary and unsavable

        old_object.save() # throws an exception; historical objects are
                          # immutable.
        
    
# === zero knowledge sharing of containers/objects with peers.
# you can share with an individual peer or with defined groups (below.) sharing
# happens by making objects available to peers or groups.  The objects continue
# to be unreadable to the server or to anyone other than the peers the objects
# are shared with.

# obligitory scenario of alice and bob wanting to communicate with crypto :)

alice = session.get_peer("alice")
if not alice:
    alert("peer alice not available")

# share an entire container (including all objects and their history since the
# last compaction) with alice.  share implies reads only, not writes.  alice
# will be able to read the historical and current state of all objects in this
# container as they are updated and changed until it is deleted or unshared.
container.share(alice)
# appropritate warnings about info theory here!
container.unshare()       # remove all sharing with all peers
container.unshare(alice)  # just remove alice
result = container.save() # still have to check for conflicts/retry, even
                          # though you're not changing any data.

# what really happens when bob unshares a container with alice? 
#  - we tell the server to no longer make the contents of the container
#    readable to alice (i.e. so that alice is disallowed by the server 
#    from retreving the encrypted data stream of the container.)
#  - we re-key the container such that all further writes to the container
#    continue to be unreadable to alice, even if alice happens to be Mallory's
#    girlfriend (and mallory is an evil system administrator employed by the
#    service operator, with administrative and physical access to the server(s)
#    the container is stored on.)
#  (information theory of course means that any data previously shared with
#  alice, alice already knows. so there's not much point in re-encrypting that
#  data.)

# share only a specific object with alice. like above, object will remain
# shared and the peer can see changes/updates/history.  this is more expensive
# than just sharing a whole container, but not excessively so.  From alice's
# perspective, it seems like the container only contains the specific objects
# that are shared.
myobject.share(alice)
myobject.unshare(alice)
myobject.save()

# how alice gets to this data (from her own session)
bob = session.get_peer("bob")
# get a container object for from peer
bob_container = bob.load("container_name")
# bob_container may have every key available to us (if bob shared the whole
# container), or just the specific keys that bob shared with us.  from alice's
# perspective, she can't tell the difference between a container that's fully
# shared vs. one that just has some shared objects in it.
bob_object = bob_container.get('bobs_object')

# now we could read bob's diary as above, just as if it was one of our own
# objects. We just can't save changes back to it.  (But more on that later...)

# === secure real time messages. this is not messages in the
# sense of email, but messages in the sense of message queues, or message
# oriented application development.  still, the inbox metaphore mostly fits.
# messages are async; the recipients don't have to be connected/online right
# now to receive these. they'll notice new messages whenever they next poll
# their inbox.

# allow our peer alice to send us stuff.
session.inbox.allow_messages(alice, 
    { max_unread_messages: 1000, 
         max_message_size: 10000, 
          max_unread_size: 1000000 })
# TODO: resolve this question: should it be possible to blanket allow messages
# from anyone? maybe a limit of just a few small messages?
# proposed answer: make it easy to allow a small number of messages from
# anyone, and have that limit automatically go away if you send a message back.

# sending and checking message queues. we can send messages to ourselves, or to
# our peers that have set allow_messages from us.
# messages have these properties: id, from, to, timestamp, size, header, body, ttl
# messages are immutable -- they are only created, delivered, retrieved, and
#   deleted.
# they are never modified (not even in the sense that they are "marked as
#   read").  In that sense, in message queue terms, they have "at least once"
#   delivery semantics.
# header and body are encrypted such that only recipient can read them.
# the size property is the length of the ciphertext of headers + body
# headers may optionally be retrieved when the inbox contents is listed.
# headers are limited to 4k in size per message.
# the headers are really just an object. any javascript JSON object is allowed.
#   the size limitation is applied to the serialized, compressed, and encrypted
#   form of the object.
# the body is also an object, but without a hard limit.
# headers and body must be defined. it's conventional to send false as a value
#   when you don't need to send a particular value.
# searching or filtering through the listed messages by headers necessarily
#   takes place on the client side, since the server cannot decrypt them for
#   us.
# headers and body together count towards maximum message sizes that a peer
#   might be willing to receive from another peer (as configured above.)
session.inbox.poll()                # return when new messages are available
session.inbox.messages_by_peer()    # get a map of peer -> number of messages
session.inbox.list()                # get list of unread messages (metadata only)
for msg in session.inbox.list():
    # properties: msg.id, msg.from, msg.to, msg.timestamp, msg.size, msg.ttl
    msg.get_headers()
    msg.get_body()
    msg.delete() # it's faster to build up a list of IDs and delete them all at
                 # once (as below.)
    tx.add(msg.delete)  # add deleting this message to a running transaction, 
                        # such that deleting the message can happen atomically
                        # in combination with changes to objects.
tx.commit() # rememer to commit our transaction

# list with filtering. all parameters optional. specifying no filtering
# parameters gets same results as list, except that the inclusion of headers in
# the result set can be controlled.
session.inbox.filter(peer = bob, 
                     after = earliest_time, 
                     before = latest_time,
                     include_headers = True,
                     header_filter = my_filter_function,
                     limit = max_messages_to_return)
session.inbox.get([list_of_message_ids])  # get a list of message headers and bodies
session.inbox.delete([list_of_message_ids]) # atomically remove a list of messages
session.inbox.clear() # flippantly delete all messages, seen or unseen

message_id = peer.send_message(header, body)
if message_id == 'error'

# sharing with groups instead of just individual peers:
# groups are about membership.
# groups members can be individuals or other groups, recursively to a
#   reasonably high limit.
# group membership always must happen by invitation, such that an existing
#   member (with read privledges) invites new members.
# a group has a creator.
# members maybe given permission (individually) to 
#  - read data intended for the group
#  - author data as the group
#  - invite new members
#  - revoke existing members
#  - give a member the privledge to invite
#  - give a member the privledge to revoke
# crypto structure for a group:
#  - a group has a set of keys like an individual
#   - read key
#   - sign key
#  - whenever a new member joins the group, the read key for the group
#    is encoded to that member's keys.
#  - when a member leaves the group, the groups keys are rotated:
#    - new group keys are generated
#    - all new keys are encrypted to all the continuing members of the group.

# - can create a group.
# - can invite others to the group as:
#     - load containers shared to group
#     # modify containers shar
#   - viewers (can load containers/messages )
#   - writers (can add new containers/messages)
#   - administrators (can change privledges of others)
#   - owners (can destroy the group)
# - peers have to accept an invitation to join a group, although they could
#   also set an auto-accept setting.
# - storage for the group object is outside the billable storage amount for any
#   particular account.

# TODO: standardize result objects. 
#   result.status, result.error, result.error_message

# TODO: expose the internal container data for sharing histroy. like, to see
# the peers you've shared, the objects you've shared with them, etc.

# TODO: add optional automatic notifications of shared object access -- where
# the system sends you messages when a peer accesses one of your shared
# objects. so apps can implement auditing requirements. (so the messages
# wouldn't originate from a peer, but from the server itself, in response to
# object retrieval.)

# TODO multi-user read-write shared containers
# TODO container compaction
