# Introduction

Crypton allows the creation of cryptographically secure web applications where the server is blind to the data being stored. This giving the user peace of mind that their data is theirs alone to see.

# Client side

On a high level, the Crypton client library is architected like the following diagram do provide transparent security:

````
  Public API
       |
 Cryptography
       |
Secure random()
    /     \
  RNG    Entropy Accumulator
````

The client side API of Crypton resembles the following hierarchy:

````
 Account  -  Inbox
    |
 Session
    |
Containers
    |
 Objects
````

## Accounts

Each user has an account with optional arbitrary plaintext data up to TODO kb. You can ask `crypton` to generate and account, which creates all necessary keyfiles:

````javascript
var handle = 'inputFromUser';
var passPhrase = 'moreInputFromUser';

crypton.generateAccount(handle, passPhrase, accountHandler);
````

### account.save(callback)

Sends account object to the server for storage, either updating or creating the record.

TODO rate limiting or some other form of not getting pwned?

````javascript
var accountHandler = function (err, account) {
  if (err) {
    // alert the user, adjust application flow
    return;
  }

  account.save(function (err) {
    if (err) {
      // alert the user, adjust application flow
      return;
    }
  
  });
});
````

## Sessions

A session is necessary for requesting and receiving data.

````javascript
crypton.authorize(handle, passPhrase, function (err, session) {
  if (err) {

    return;
  }

  
});
````

### session.serialize(callback)

Serialize a session for storage

````javascript
session.serialize(function (err, sessionString) {
  // store the data somewhere
});
````

### session.ping(callback)

After resurrecting a cached session, we will want to make sure the server still beleives that it is valid. For example the session may be invalid is the password has been changed since it was saved, if the account has been deleted, or if it has been disabled server-side (such as for non-payment).

````javascript
crypton.resurrect(sessionString, function (err, session) {
  session.ping(function (err) {
    if (err) {
      // alert the user
      return;
    }

    // continue application flow
  });
});
````

### session.load(containerName, callback)
### session.create(containerName, callback)

See below

## Containers

Data in Crypton is treated as a traditional object database. Containers are append-only stores that are transparently encrypted on the client side.

Containers are identified by keys such as "diary" in the example below. Any string is allowed. From the server's perspective, container names (and of course their contents) are unreadable.

If the data set is going to grow very large, partitioning data across containers and lazy loading them as you need them can help with app load time. One simple tactic for partitioning data among containers is to keep app metadata all in one container, and keep bulky binary data (images, videos, long text strings, whatever) in many other containers. Metadata is usually very small, and can load quickly.

For example, a diary app might have a single metadata container with a list of entries. Each entry has a title, date, keywords, and other basic attributes. Average size is probably under 500 bytes. So one entry per day for 10 years would only be be 1.8 meg. In 10 years, 1.8 meg will be like 1.8 KB today, so storing all this in one container is fine. Each entry would reference other containers to find the text of the entry and media attachments like images, videos, etc.

````javascript
session.load('diary', function (err, diary) {
  // diary is a container, specific to the current account
});
````

### container.get(objectName, callback)

Retreive and object from said container and transparently decrypt it

````javascript
container.get('entries', function (err, diaryEntries) {
  if (err) {
    // alert the user, adjust the application flow
    return;
  }

  window.diaryEntries = diaryEntries;
});

container.get('drafts', function (err, diaryEntries) {
  if (err) {
    // alert the user, adjust the application flow
    return;
  }

  window.diaryDrafts = diaryDrafts;
});
````

### container.save(callback)

````javascript
var newEntry = {
  id: diaryEntries.length + diaryDrafts.length,
  title: 'Adventures with Crypto'
};

diaryDrafts.push(newEntry);

// Atomically save all modified objects back to the container. If we had made
// changes to either diaryEntries or diaryDrafts, both would be saved. The
// default parameters for saving objects preserves object history (i.e. previous
// versions of the object are still reachable) and uses diffing where
// appropritae to minimize the total size. 
diaryDrafts.save(function (err) {
  if (err) {
    // alert the user
    return;
  }

});
````

### container.add(key, value);

Let's add some more content to this entry. We'll store the text content separately from the metadata, in its own one-off container.

````javascript
var text = 'Dear diary,\n\nToday, I transparently encrypted data with the RSA algorithm and it was dreamy.';

newEntry.textContainerName = 'textFor' + newEntry.id;
session.newContainer(newEntry.textContainerName, function (err, textContainer) {
  textContainer.add(newEntry.id, text);
  textContainer.save(function (err) {
    if (err) {
      // alert the user
      return;
    }

  });
});
````

## Objects

## Inbox

## Groups

# Server side

