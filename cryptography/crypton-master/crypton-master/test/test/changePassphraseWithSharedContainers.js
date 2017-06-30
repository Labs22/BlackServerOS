/* Crypton Server, Copyright 2014 SpiderOak, Inc.
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

describe('Change passphrase after sharing containers', function () {
  this.timeout(200000);

  var alice;
  var bob;
  var aliceSession;
  var bobSession;
  var alicePeer;
  var bobPeer;
  var bob2AliceContainerHmacName;
  var alice2BobContainerHmacName;


  describe('Create 2 accounts', function () {
    it('Create Alice', function (done) {
      crypton.generateAccount('alice2', 'pass', function (err, acct) {
        if (err) throw err;
        alice = acct;
        done();
      });
    });

    it('Get Alice\'s session', function (done) {
      crypton.authorize('alice2', 'pass', function (err, sess) {
        if (err) throw err;
        aliceSession = sess;
        done();
      });
    });

    it('Create Bob', function (done) {
      crypton.generateAccount('bob2', 'pass', function (err, acct) {
        if (err) throw err;
        bob = acct;
        done();
      });
    });

    it('Get Bob\'s session', function (done) {
      crypton.authorize('bob2', 'pass', function (err, sess) {
        if (err) throw err;
        bobSession = sess;
        done();
      });
    });

    it('Get Alice as peer', function (done) {
      bobSession.getPeer('alice2', function (err, peer) {
        assert.equal(err, null);
        if (err) throw err;
        peer.trust(function (err) {
          assert.equal(err, null);
          if (err) throw err;
          alicePeer = peer;
          done();
        });
      });
    });

    it('Create a container and share it with Alice', function (done) {
      bobSession.create('bob-to-alice-container', function (err, container) {
        assert.equal(err, null);
        if (err) {
          throw(err);
        }
        // add data to the container
        container.add('toast', function (err) {
          assert.equal(err, null);
          if (err) {
            throw(err);
          }
          container.keys['toast'].landed = 'Butterside Up!';
          container.save(function (err) {
            assert.equal(err, null);
            bob2AliceContainerHmacName = container.getPublicName();
            done();
          });
          // Need to share this conatiner with Alice
          container.share(alicePeer, function (err) {
            assert.equal(err, null);
            if (err) {
              done();
            }
          });
        }); // end conatiner.add
      });
    });

    it('Alice needs to login again so Alice\'s session is valid', function (done) {
      crypton.authorize('alice2', 'pass', function (err, sess) {
        assert.equal(err, null);
        assert(sess);
        if (err) throw err;
        aliceSession = sess;
        done();
      });
    });

    it('Get Bob as peer', function (done) {
      aliceSession.getPeer('bob2', function (err, peer) {
        assert.equal(err, null);
        assert(peer);
        if (err) throw err;
        peer.trust(function (err) {
          assert.equal(err, null);
          if (err) throw err;
          bobPeer = peer;
          done();
        });
      });
    });

    // Alice should create a container to share with Bob
    it('Create a container and share it with Bob', function (done) {
      aliceSession.create('alice-to-bob-container', function (err, container) {
        assert.equal(err, null);
        if (err) {
          throw(err);
        }
        // add data to the container
        container.add('frenchtoast', function (err) {
          assert.equal(err, null);
          if (err) {
            throw(err);
          }
          container.keys['frenchtoast'].smothered = 'With Maple Syrup!';
          container.save(function (err) {
            assert.equal(err, null);
            if (err) {
              throw(err);
            }
            alice2BobContainerHmacName = container.getPublicName();
            // Need to share this container with Bob
            container.share(bobPeer, function (err) {
              assert.equal(err, null);
              if (err) {
                throw(err);
              }
              done();
            });
          });
        }); // end conatiner.add
      });
    });

    // Alice should change her password
    it('Change Alice\'s passphrase', function (done) {
      assert.equal(typeof aliceSession, 'object');
      function cb (err, isComplete) {
        assert.equal(err, null);
        assert.equal(isComplete, true);
        if (err) {
          throw(err);
        }
        assert.equal(isComplete, true);
        done();
      }

      function uiCb () {
        // noop
      }

      try {
        aliceSession.account.changePassphrase('pass', 'foobarstrongerpass', cb, uiCb, false);
      } catch (ex) {
        console.error(ex);
        console.error(ex.stack);
        done();
      }
    });

    // Alice should re-login & load the container via the hmac name
    it('Alice re-login & load shared container from Bob', function (done) {
      crypton.authorize('alice2', 'foobarstrongerpass', function (err, newSession) {
        assert.equal(err, null);
        if (err) {
          throw(err);
        }
        assert.equal(newSession.account.username, 'alice2');
        // Get bobPeer again
        newSession.getPeer('bob2', function (err, peer) {
          assert.equal(err, null);
          assert.equal(peer.trusted, true);
          // Lets check the content of our container
          newSession.loadWithHmac(bob2AliceContainerHmacName, peer, function (err, container) {
            assert.equal(err, null);
            assert(container);
            assert.equal(container.keys['toast'].landed, 'Butterside Up!');
            done();
          });
        });
      });
    });

    // Bob login again
    it('Get Bob\'s session', function (done) {
      crypton.authorize('bob2', 'pass', function (err, sess) {
        if (err) throw err;
        bobSession = sess;
        done();
      });
    });

    // Bob should change his passphrase
    it('Change Bob\'s passphrase', function (done) {
      assert.equal(typeof bobSession, 'object');
      function cb (err, isComplete) {
        assert.equal(err, null);
        assert.equal(isComplete, true);
        if (err) {
          throw(err);
        }
        assert.equal(isComplete, true);
        done();
      }

      function uiCb () {
        // noop
      }

      try {
        bobSession.account.changePassphrase('pass', 'foobarstrongerpass', cb, uiCb, false);
      } catch (ex) {
        console.error(ex);
        console.error(ex.stack);
        done();
      }
    });

    // Bob should load the shared container and read it
    it('Bob re-login & load shared container from Alice', function (done) {
      crypton.authorize('bob2', 'foobarstrongerpass', function (err, newSession) {
        assert.equal(err, null);
        if (err) {
          throw(err);
        }
        assert.equal(newSession.account.username, 'bob2');
        // Get bobPeer again
        newSession.getPeer('alice2', function (err, peer) {
          assert.equal(err, null);
          assert.equal(peer.trusted, true);
          // Lets check the content of our container
          newSession.loadWithHmac(alice2BobContainerHmacName, peer, function (err, container) {
            assert.equal(err, null);
            assert(container);
            assert.equal(container.keys['frenchtoast'].smothered, 'With Maple Syrup!');
            done();
          });
        });
      });
    }); // end Bob load shared container

  });
});
