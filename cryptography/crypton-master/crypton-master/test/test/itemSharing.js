/* Crypton Server, Copyright 2015 SpiderOak, Inc.
 *
 * This file is part of Crypton Server.
 *
 * Crypton Server is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Server is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Server.  If not, see <http://www.gnu.org/licenses/>.
*/

describe('Item sharing tests', function () {
  this.timeout(200000);

  var alice, bob;
  var aliceSession, bobSession;
  var itemNameHmac;
  var item1;

  describe('Create Account', function () {
    it('Create Alice', function (done) {
      crypton.generateAccount('alice4', 'pass', function (err, acct) {
        if (err) throw err;
        assert(acct);
        console.log(acct);
        alice = acct;
      });

      crypton.generateAccount('bob4', 'pass', function (err, acct) {
        if (err) throw err;
        assert(acct);
        bob = acct;
        console.log(acct);
        done();
      });

    });

    it('Get Bob\'s session', function (done) {
      crypton.authorize('bob4', 'pass', function (err, sess) {
        if (err) throw err;
        bobSession = sess;
        assert(sess);
        console.log(sess);
        bobSession.events['message'] = function (message) {
          console.log('message rcvd: ', message);
        };
        // trust alice
        bobSession.getPeer('alice4', function (err, peer) {
          if (err) { throw err };
          peer.trust(function (err) {
            if (err) throw err;
            assert(peer.trusted);
            // XXXddahl: need to clear out the entire message inbox

            done();
          });
        });
      });
    });

    it('Get Alice\'s session', function (done) {
      crypton.authorize('alice4', 'pass', function (err, sess) {
        if (err) throw err;
        aliceSession = sess;
        assert(sess);
        console.log(sess);
        // trust bob
        aliceSession.getPeer('bob4', function (err, peer) {
          if (err) { throw err };
          peer.trust(function (err) {
            if (err) throw err;
            assert(peer.trusted);
            done();
          });
        });
      });
    });

    it('Create an Item', function (done) {
      aliceSession.getOrCreateItem('my-first-shared-item', function (err, item) {
        if (err) {
          console.error(err);
          throw (err);
        }
        console.log(item);
        item1 = item;
        assert(item);
        assert(item.sessionKey);
        assert(item.value);
        itemNameHmac = item.nameHmac;
        done();
      });
    });


    it('Update Item', function (done) {
      try {
        item1.value = { foo: 1, bar: 2, baz: 3 };
      } catch (ex) {
        console.error(ex);
        throw new Error(ex);
      }
      done();
    });

    it('share item with bob', function (done) {
      var bobPeer = aliceSession.peers.bob4;
      assert.equal(bobPeer.username, 'bob4');
      item1.share(bobPeer, function (err) {
        if (err) throw err;
        assert.equal(null, err);
        done();
      });
    });

    // check that an item was shared
    it('check item was shared', function (done) {
      crypton.authorize('bob4', 'pass', function (err, sess) {
        if (err) throw err;
        bobSession = sess;
        assert(sess);
        bobSession.inbox.getAllMetadata(function (err, messageList) {
          assert.equal(messageList.length, 1);
          // Get and decrypt message:
          bobSession.inbox.get(messageList[0].messageId,
            function (err, message) {
              if (err) throw err;
              assert.equal(message.headers.notification, 'sharedItem');
              assert.equal(message.payload.from, 'alice4');
              assert.equal(message.payload.itemNameHmac, item1.nameHmac);
              assert(message.payload.sent);
              // Load the shared Item
              bobSession.getSharedItem(message.payload.itemNameHmac,
                bobSession.peers.alice4,
                function (err, item) {
                  if (err) throw err;
                  assert.equal(item.value.baz, 3);
                  done();
                });
            });
        });
      });
    });

    it('unshare item with bob', function (done) {
      // authorize Alice
      crypton.authorize('alice4', 'pass', function (err, sess) {
        if (err) throw err;
        aliceSession = sess;
        assert(sess);
        // trust bob
        aliceSession.getPeer('bob4', function (err, peer) {
          if (err) { throw err };
          aliceSession.getOrCreateItem('my-first-shared-item',
          function (err, item) {
            if (err) { throw err };
            item.unshare(peer, function (err) {
              assert.equal(err, null);
              done();
            });
          });
        });
      });
    });

    it('Make sure bob cannot access unshared item', function (done) {
      crypton.authorize('bob4', 'pass', function (err, sess) {
        if (err) throw err;
        bobSession = sess;
        bobSession.getPeer('alice4', function (err, peer) {
          if (err) { throw err };
          bobSession.getSharedItem(itemNameHmac, peer, function (err, item) {
            assert(err);
            assert.equal(item, null);
            done();
          });
        });
      });
    });

  });
});
